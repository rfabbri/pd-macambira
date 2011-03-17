local P = {}
if _REQUIREDNAME == nil then
  complex = P
else
  _G[_REQUIREDNAME] = P
end

-- imports
-- local sqrt = math.sqrt

-- no more external access after this point
setfenv(1, P)

function new (r, i) return {r=r, i=i} end

i = new(0, 1)

function add(c1, c2)
  return new(c1.r + c2.r, c1.i + c2.i)
end

function sub(c1, c2)
  return new(c1.r - c2.r, c1.i - c2.i)
end

function mul(c1, c2)
  return new(c1.r * c2.r - c1.i * c2.i, c1.r * c2.i + c1.i * c2.r)
end

return P
