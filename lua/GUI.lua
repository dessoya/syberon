
local GUI = { }

-- virtual objects

GUI.Object = require("GUI\\Object")
GUI.Hover = require("GUI\\Hover")
GUI.HoverPush = require("GUI\\HoverPush")
GUI.Fonts = require("GUI\\Fonts")

-- real render object

GUI.Rect = require("GUI\\Rect")
GUI.Text = require("GUI\\Text")
GUI.ImageText = require("GUI\\ImageText")
GUI.Image = require("GUI\\Image")

-- complex object

GUI.Background = require("GUI\\Background")
GUI.Window = require("GUI\\Window")
GUI.Button = require("GUI\\Button")
GUI.ProgressBar = require("GUI\\ProgressBar")
GUI.ComboBox = require("GUI\\ComboBox")
GUI.CheckBox = require("GUI\\CheckBox")
GUI.RadioBox = require("GUI\\RadioBox")

return GUI
