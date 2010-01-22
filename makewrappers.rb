lb_Windows = RUBY_PLATFORM.downcase.include?('mswin')
lb_Mac = RUBY_PLATFORM.downcase.include?('darwin')
print 'Building wrappers...'
ls_Tool = 'Proteomatic'
ls_ToolCore = ls_Tool + 'Core'
ls_Tool += '.exe' if lb_Windows
ls_ToolCore += '.exe' if lb_Windows
ls_Res = ''
ls_Res = "obj/release/#{ls_Tool.sub('.exe', '')}_res.o -mwindows" if lb_Windows
system("gcc -o #{ls_Tool} -DBINARY=\\\"#{ls_Tool}\\\" -DBINARYCORE=\\\"#{ls_ToolCore}\\\" -D#{lb_Windows ? "WIN32" : "WHATEVER"} src/BinaryWrapper.cpp #{ls_Res} -lstdc++")
if lb_Mac
	system("strip #{ls_Tool}")
else
	system("strip --strip-all #{ls_Tool}")
end
puts 'done.'
