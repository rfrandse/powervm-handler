#include "pldm_oem_cmds.hpp"

#include "pldm_utils.hpp"
#include "xyz/openbmc_project/Common/error.hpp"

#include <fmt/core.h>
#include <libpldm/base.h>
#include <libpldm/platform.h>
#include <unistd.h>

#include <fstream>
#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/log.hpp>
#include <sdbusplus/bus.hpp>

namespace openpower::dump::pldm
{
using namespace phosphor::logging;

constexpr auto eidPath = "/usr/share/pldm/host_eid";
constexpr mctp_eid_t defaultEIDValue = 9;

using NotAllowed = sdbusplus::xyz::openbmc_project::Common::Error::NotAllowed;
using Reason = xyz::openbmc_project::Common::NotAllowed::REASON;

namespace internal
{
mctp_eid_t readEID()
{
    mctp_eid_t eid(defaultEIDValue);

    std::ifstream eidFile{eidPath};
    if (!eidFile.good())
    {
        log<level::ERR>("Could not open host EID file");
        elog<NotAllowed>(Reason("Required host dump action via pldm is not "
                                "allowed due to mctp end point read failed"));
    }
    else
    {
        std::string strEid;
        eidFile >> strEid;
        if (!strEid.empty())
        {
            eid = strtol(strEid.c_str(), nullptr, 10);
        }
        else
        {
            log<level::ERR>("EID file was empty");
            elog<NotAllowed>(
                Reason("Required host dump action via pldm is not "
                       "allowed due to mctp end point read failed"));
        }
    }

    return eid;
}
} // namespace internal

void ackNewDump(uint32_t dumpId, pldm_fileio_file_type pldmDumpType,
                uint64_t dumpSize)
{
    const size_t pldmMsgHdrSize = sizeof(pldm_msg_hdr);
    std::array<uint8_t, pldmMsgHdrSize + PLDM_FILE_ACK_REQ_BYTES> fileAckReqMsg;

    mctp_eid_t mctpEndPointId = internal::readEID();

    auto pldmInstanceId = getPLDMInstanceID(mctpEndPointId);

    int retCode =
        encode_new_file_req(pldmInstanceId, pldmDumpType, dumpId, dumpSize,
                            reinterpret_cast<pldm_msg*>(fileAckReqMsg.data()));
    if (retCode != PLDM_SUCCESS)
    {
        log<level::ERR>(
            fmt::format("Failed to encode pldm FileAck for new dump available "
                        "dumpId({}), "
                        "pldmDumpType({}),rc({})",
                        dumpId, pldmDumpType, retCode)
                .c_str());
        elog<NotAllowed>(
            Reason("Acknowledging new file failed due to encoding error"));
    }

    uint8_t* pldmRespMsg = nullptr;
    size_t pldmRespMsgSize;

    internal::CustomFd pldmFd(openPLDM());

    retCode =
        pldm_send_recv(mctpEndPointId, pldmFd(), fileAckReqMsg.data(),
                       fileAckReqMsg.size(), &pldmRespMsg, &pldmRespMsgSize);

    std::unique_ptr<uint8_t, decltype(std::free)*> pldmRespMsgPtr{pldmRespMsg,
                                                                  std::free};
    if (retCode != PLDM_REQUESTER_SUCCESS)
    {
        auto errorNumber = errno;
        log<level::ERR>(
            fmt::format("Failed to send pldm FileAck for new dump available, "
                        "dumpId({}), pldmDumpType({}), "
                        "rc({}), errno({}), errmsg({})",
                        dumpId, pldmDumpType, retCode, errorNumber,
                        strerror(errorNumber))
                .c_str());
        elog<NotAllowed>(Reason("New file available  via pldm is not "
                                "allowed due to fileack send failed"));
    }

    uint8_t completionCode;

    retCode =
        decode_new_file_resp(reinterpret_cast<pldm_msg*>(pldmRespMsgPtr.get()),
                             pldmRespMsgSize - pldmMsgHdrSize, &completionCode);
    if (retCode || completionCode)
    {
        log<level::ERR>(
            fmt::format("Failed to send new file available, dumpId({}), "
                        "pldmDumpType({}), rc({}), "
                        "completionCode({})",
                        dumpId, pldmDumpType, retCode, completionCode)
                .c_str());
        elog<NotAllowed>(Reason("Sendinf new file available via pldm is "
                                "failed"));
    }

    log<level::INFO>(
        fmt::format("Acknowledged PHYP about new dump, dumpId({})", dumpId)
            .c_str());
}
} // namespace openpower::dump::pldm
