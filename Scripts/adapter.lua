local HOST = 'localhost'
local PORT = 13249

function connect()
  s = socket.tcp(HOST, PORT)
  if s == nil then
    print('Failed to connect to ' .. HOST .. ':' .. PORT)
    socket.sleep(1)
    return connect()
  end
  return s
end

local s = connect()

print("Connected to the client at " .. HOST .. ":" .. PORT)

while true do
  local op = binary.unpack_u8(s:recv(1))

  -- MEMREAD_8
  if op == 2 then
    local addr = binary.unpack_u32(s:recv(4))
    local data = memory.read_u8(addr)
    s:send(binary.pack_u8(data))
  end

  -- MEMREAD_16
  if op == 3 then
    local addr = binary.unpack_u32(s:recv(4))
    local data = memory.read_u16(addr)
    s:send(binary.pack_u16(data))
  end

  -- MEMREAD_32
  if op == 4 then
    local addr = binary.unpack_u32(s:recv(4))
    local data = memory.read_u32(addr)
    s:send(binary.pack_u32(data))
  end

  -- MEMWRITE_8
  if op == 6 then
    local addr = binary.unpack_u32(s:recv(4))
    local data = binary.unpack_u8(s:recv(1))
    memory.write_u8(addr, data)
  end

  -- MEMWRITE_16
  if op == 7 then
    local addr = binary.unpack_u32(s:recv(4))
    local data = binary.unpack_u16(s:recv(2))
    memory.write_u16(addr, data)
  end

  -- MEMWRITE_32
  if op == 8 then
    local addr = binary.unpack_u32(s:recv(4))
    local data = binary.unpack_u32(s:recv(4))
    memory.write_u32(addr, data)
  end
end
