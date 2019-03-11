-- Gets a string representation of list.
-- local function listToString (list)
--    local t = {}
--    for _,v in ipairs (list) do
--       table.insert (t, tostring (v))
--    end
--    return table.concat (t, ', ')
-- end

do
   local mt = ...

   mt.__index = function (self, k)
      return mt[k] or mt[self][k]
   end

   mt.__newindex = function (self, k, v)
      mt[self][k] = v
   end

   -- Attaches private data and access functions.
   mt._attachData = function (self, data, funcs)
      local data = data or {}
      local funcs = funcs or {}
      local _mt = {}
      _mt.__index = function (_, k)
         local t = funcs[k]
         if t == nil then
            return nil
         end
         local f = t[1]
         if f == nil then
            return nil
         end
         return f (self)
      end
      _mt.__newindex = function (_, k, v)
         local t = funcs[k]
         if t == nil then
            return
         end
         local f = t[2]
         if f == nil then
            return
         end
         fset (self, v)
      end
      mt[self] = setmetatable (data, _mt)
      mt._init (self)
   end

   -- Detaches private data.
   mt._detachData = function (self)
      mt._fini (self)
      mt[self] = nil
   end

   -- Initializes private data.
   mt._init = function ()
   end

   -- Finalizes private data.
   mt._fini = function ()
   end

   -- Trace.
   -- for k,f in pairs (mt) do
   --    if k:sub (1,2) ~= '__' and type (f) == 'function' then
   --       mt[k] = function (...)
   --          local fin = {...}
   --          local fout = {f (...)}
   --          if #fout > 0 then
   --             print (('%s: %s (%s) -> %s')
   --                   :format (mt.__name, k, listToString (fin),
   --                            listToString (fout)))
   --          else
   --             print (('%s: %s (%s)')
   --                   :format (mt.__name, k, listToString (fin)))
   --          end
   --          return table.unpack (fout)
   --       end
   --    end
   -- end
end
