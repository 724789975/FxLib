--设置lua的路径 这样 就不会有文件加载不到了
local p = "./scripts/"  
local m_package_path = package.path  
package.path = string.format("%s;%s?.lua;%s?/init.lua", m_package_path, p, p) 
