require 'yaml'

lk_Palette = YAML::load_file('tango-palette.yaml')

puts '<html>'
puts '<head>'
puts '<title>Tango Palette</title>'
puts '</head>'
puts '<body>'
puts "<table style='font-family: monospace;'>"
lk_Palette.each do |ls_Name|
	print "<tr><td>#{ls_Name.keys.first}</td><td style='padding: 8px;'>"
	ls_Name.values.first.each do |ls_Color|
		print "<span style='padding: 8px; background-color: ##{ls_Color}'>##{ls_Color}</span>"
	end
	puts "</td></tr>"
end
puts '</table>'
puts '</body>'
puts '</html>'