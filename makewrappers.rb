lb_Windows = RUBY_PLATFORM.downcase.include?('mswin')
print 'Building wrappers...'
['Proteomatic', 'ProteomaticPipeline', 'Revelio'].each do |ls_Tool|
	ls_Tool += '.exe' if lb_Windows
	system("gcc -o #{ls_Tool} -DBINARY=\\\"#{ls_Tool}\\\" -D#{lb_Windows ? "WIN32" : "WHATEVER"} src/src/BinaryWrapper.cpp -lstdc++")
	system("strip --strip-all #{ls_Tool}")
end
puts 'done.'
