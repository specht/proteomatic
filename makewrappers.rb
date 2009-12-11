lb_Windows = RUBY_PLATFORM.downcase.include?('mswin')
print 'Building wrappers...'
['Proteomatic'].each do |ls_Tool|
	ls_Tool += '.exe' if lb_Windows
	ls_Res = ''
	ls_Res = "obj/release/#{ls_Tool.sub('.exe', '')}_res.o" if lb_Windows
	system("gcc -o #{ls_Tool} -DBINARY=\\\"#{ls_Tool}\\\" -D#{lb_Windows ? "WIN32" : "WHATEVER"} src/BinaryWrapper.cpp #{ls_Res} -lstdc++")
	system("strip --strip-all #{ls_Tool}")
end
puts 'done.'
