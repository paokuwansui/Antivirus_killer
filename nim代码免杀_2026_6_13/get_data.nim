import std/[os, strutils, random]
import ./decrypt

proc getRandomChar(char_set: string): char =
  let randomIndex = rand(0 ..< charset.len)
  return charset[randomIndex]

proc generateRandomByStr(lengthStr: string, char_set: string): string =
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
    result.add(getRandomChar(char_set))

proc get_key(key: string, len_num: string, data: string, check_data: string, char_set: string): string = 
  var splice_key = ""
  while true:
    splice_key = generateRandomByStr(len_num, char_set) & key
    let out_encrypt = decrypt.decrypt_data(check_data, splice_key, data)
    if out_encrypt == data:
      break
  return splice_key

proc get_data*(fileName: string = "payload.txt"): tuple[key: string, b85chars: string, check_data: string, data: string] =
  if not fileExists(fileName): 
    return ("", "", "", "")
  let rawContent = readFile(fileName)
  let lines = rawContent.splitLines()
  if lines.len < 3:
    echo "[-] 错误: payload.txt 内容行数不足 3 行！"
    return ("", "", "", "")
  let flag = $lines[0][0]
  var key = lines[0][1..^1]
  let b85chars = lines[1][0..84]
  let charset = lines[1][85..^1]
  let thirdLine = lines[2]
  var check_data = ""
  var data = ""
  if thirdLine.len >= 120:
    check_data = thirdLine[0..<120]
    data = thirdLine[120..^1]
  else:
    check_data = thirdLine
    echo "[!] 警告: 第三行数据长度不足 120 字节！"

  # 爆破还原密钥
  key = get_key(key, flag, b85chars, check_data, charset)
  return (key, b85chars, check_data, data)

proc get_payload*(data: string, key: string, b85chars: string): string =
    return decrypt.decrypt_data(data, key, b85chars)