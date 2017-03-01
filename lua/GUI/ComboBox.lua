local HoverPush = require("GUI\\HoverPush")
local Fonts = require("GUI\\Fonts")
local Rect = require("GUI\\Rect")
local Text = require("GUI\\Text")

local ComboBoxItem = HoverPush:extend()
function ComboBoxItem:initialize(x, y, w, caption, cb)

	HoverPush.initialize(self, x, y, w, 25, cb)
	self.id = "comboboxitem_" .. self:getIDFor("comboboxitem")

	self.caption = caption

	self.bx = x
	self.by = y

	self.rect = self:addChild(Rect:new(self.x, self.y, self.w, 25, 100, 100, 100))
	self.text = self:addChild(Text:new(self.x + 3, self.y + 3 , caption, Fonts.basic, 255, 255, 255))

end

function ComboBoxItem:onHoverStatesChange()
	local c
	if self.hover then c = 150 else c = 100 end
	self.rect.r = c
	self.rect.g = c
	self.rect.b = c
	self.rect:setProp()
end

function ComboBoxItem:onParentChangePosition(parent)

	self.x = parent.x + self.bx
	self.y = parent.y + self.by

	self.rect.x = self.x
	self.rect.y = self.y
	self.rect:setProp()
	
	self.text.x = self.x + 3
	self.text.y = self.y + 3
	self.text:setProp()
	
	return true

end

function ComboBoxItem:onPushStateChange()
end

local ComboBox = HoverPush:extend()

function ComboBox:initialize(x, y, w, rend, cb)

	self.cb = cb
	self.list = { }
	self.rend = rend

	self.listItems = { }

	HoverPush.initialize(self, x, y, w, 40, function()
		
		self:hideList()

		self.listItems = { }

		-- drop down
		self.rend:lockObjectList()
		for k, v in ipairs(self.list) do
			local l = self:addChild(ComboBoxItem:new(0, k * 27, self.w, v.caption, function(item)
				self:setupItem(item.__id)
				self:hideList()
			end))
			l.__id = k
			l:onParentChangePosition(self)
			table.insert(self.listItems, l)
			l:pushToRenderer(rend)
		end
		self.rend:unlockObjectList()


		self.comboOpen = true

	end)

	self.comboOpen = false
	
	self.bx = x
	self.by = y
	self.id = "combobox_" .. self:getIDFor("combobox")

	self.caption = self:addChild(Text:new(x + 3, y + 3, "", Fonts.basic, 255, 255, 255))
	--[[
	self.__w = list[id].w
	self.__h = list[id].h
	]]

end

function ComboBox:hideList()

	if table.getn(self.listItems) == 0 then
		return
	end

	self.rend:lockObjectList()
	for k, v in ipairs(self.listItems) do
		v:delFromRenderer(self.rend)
		self:delChild(v)
	end
	self.listItems = { }
	self.rend:unlockObjectList()
end

function ComboBox:onParentChangePosition(parent)

	self.x = parent.x + self.bx
	self.y = parent.y + self.by

	self.caption.x = self.x + 3
	self.caption.y = self.y + 3
	self.caption:setProp()

	self:invokeChilds("onParentChangePosition", self)

	return true

end

function ComboBox:onPushStateChange()
end

function ComboBox:setupList(list, id)
	self.list = list
	self:setupItem(id)
end

function ComboBox:setupItem(id)	
	self.caption.text = self.list[id].caption
	self.__id = id
	self.selectedItem = self.list[id]
	self.caption:setProp()
end

return ComboBox
