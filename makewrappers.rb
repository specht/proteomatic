ls_OldDir = Dir.pwd

print 'Building wrappers...'
Dir.chdir('src/src')
['Proteomatic', 'ProteomaticPipeline', 'Revelio'].each do |ls_Tool|
	ls_Tool += '.exe' if RUBY_PLATFORM.downcase.include?('mswin')
	system("gcc -o ../../#{ls_Tool} -DBINARY=\\\"#{ls_Tool}\\\" BinaryWrapper.cpp -lstdc++")
end
puts 'done.'

Dir.chdir(ls_OldDir)
