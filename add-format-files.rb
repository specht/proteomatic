puts "Updating Proteomatic.qrc..."

system("git submodule update")

files = Dir['./src/ext/cli-tools-atlas/formats/*']
files.collect! do |x|
    "  <file>#{x.sub('./src/', '')}</file>"
end

qrc = File::read('src/Proteomatic_template.qrc')

qrc.sub!(/<!-- FORMAT FILES -->.+<!-- FORMAT FILES -->/m, 
         "<!-- FORMAT FILES -->\n" + files.join("\n") + "\n<!-- FORMAT FILES -->")

File::open('src/Proteomatic.qrc', 'w') { |f| f.puts qrc }
