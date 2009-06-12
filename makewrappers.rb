ls_OldDir = Dir.pwd

print 'Building wrappers...'
Dir.chdir('src/src')
['Proteomatic', 'ProteomaticPipeline', 'Revelio'].each do |ls_Tool|
	system("gcc -o ../../#{ls_Tool} -DBINARY=\\\"#{ls_Tool}\\\" BinaryWrapper.cpp -lstdc++")
end
puts 'done.'

Dir.chdir(ls_OldDir)
