complex = require("complex")
sqlite3 = require("luasql.sqlite3") -- evil hack, below is lame

local R = pd.Class:new():register("requirer")

function R:initialize(sel, atoms)
  if type(atoms[1]) ~= "string" then return false end
  -- require(atoms[1]) -- will this ever work?
  for k,v in pairs(_G[atoms[1]]) do
    pd.post(atoms[1].. "." .. tostring(k) .. " = " .. tostring(v))
  end
  self.inlets = 0
  self.outlets = 0
  return true
end
