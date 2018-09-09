---
--- luashell.lua
---

local canecho = true

function on(command)
   if command == echo then
      canecho = true
   end
end

function off(command)
   if command == echo then
      canecho = false
   end
end

function echo(...)
   if canecho then
      return print(unpack(arg))
   else
      return nil
   end
end

function exec(...)
   local cmdline = ""

   for i, v in ipairs(arg) do
      cmdline = cmdline .. tostring(v) .. " "
   end

   return luashell.exec(cmdline)
end

function exit()
   return luashell.exit()
end
