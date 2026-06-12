import winim
import std/[os, monotimes, times]
import system


proc checkSandboxSleep*(): bool =
  let startTime = getMonoTime()
  sleep(5000)
  let endTime = getMonoTime()
  let elapsedSeconds = (endTime - startTime).inSeconds
  if elapsedSeconds < 3:
    result = true
  else:
    result = false


proc checkForDebugger(): bool =
    winimConverterBOOLToBoolean(IsDebuggerPresent())

proc IsDomainJoined(): bool =
    var joined = false
    var lpNameBuffer: LPWSTR
    lpNameBuffer = nil
    var joinStatus: NETSETUP_JOIN_STATUS
    joinStatus = netSetupUnknownStatus
    var status: NET_API_STATUS
    status = NetGetJoinInformation(lpNameBuffer, &lpNameBuffer, &joinStatus)
    if status == NERR_Success:
        joined = joinStatus == netSetupDomainName

    if lpNameBuffer != nil:
        NetApiBufferFree(lpNameBuffer)

    return joined

proc checkdomain*(): bool =
    var result = IsDomainJoined()
    if not result:
        return true
    return false
proc isDebuggerPresent*(): bool =
    let debuggerIsDetected = checkForDebugger()
    if debuggerIsDetected:
        return true
    return false
