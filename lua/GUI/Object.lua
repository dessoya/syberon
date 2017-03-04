
local Object = require("Object")
local GUIConst = require("GUI\\Const")

local GUIObject = Object:extend()


function GUIObject:invokeMethod(methodName, ...)
	if self[methodName] ~= nil then self[methodName](self, ...) end
end

function GUIObject:invokeChilds(methodName, ...)

	if self.childs == nil then return end

	for index, object in ipairs(self.childs) do
		local ic = true
		if object[methodName] ~= nil then
			if object[methodName](object, ...) then
				ic = false
			end
		end
		if ic then object:invokeChilds(methodName, ...) end
	end

end

function GUIObject:pushToRenderer(renderer, layer)

	if self._ptr ~= nil then
		if layer == nil then layer = GUIConst.Layers.Interface end
		C_Renderer_add(renderer._ptr, self._ptr, layer)
	end

	if self.childs then
		for index, value in ipairs(self.childs) do
			value:pushToRenderer(renderer)
		end
	end
end

function GUIObject:delFromRenderer(renderer)

	if self._ptr ~= nil then
		C_Renderer_del(renderer._ptr, self._ptr, GUIConst.Layers.Interface)
	end

	if self.childs then
		for index, value in ipairs(self.childs) do
			value:delFromRenderer(renderer)
		end
	end
end


function GUIObject:getID()
	return C_GUI_Object_getID(self._ptr)
end

function GUIObject:addChild(child)
	lprint("add id:" .. child.id .. " to id:" .. self.id)
	if 	self.childs == nil then
		self.childs = {}
		self.childCount = 0
	end
	self.childCount = self.childCount + 1
	table.insert(self.childs, child)
	return child
end

function GUIObject:delChild(child)
	if self.childs ~= nil then
		for index, value in pairs(self.childs) do
			if value.id == child.id then
				table.remove(self.childs, index)
				return
			end
		end
	end
end

local ids = { }
function GUIObject:getIDFor(name)
	if ids[name] == nil then
		ids[name] = 1
	end
	local id = ids[name]
	ids[name] = ids[name] + 1
	return id
end

return GUIObject
