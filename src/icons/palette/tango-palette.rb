require 'yaml'

lk_Palette = YAML::load_file('tango-palette.yaml')

def mix(a, b, amount)
    rA = Integer('0x' + a[1, 2]).to_f / 255.0
    gA = Integer('0x' + a[3, 2]).to_f / 255.0
    bA = Integer('0x' + a[5, 2]).to_f / 255.0
    rB = Integer('0x' + b[1, 2]).to_f / 255.0
    gB = Integer('0x' + b[3, 2]).to_f / 255.0
    bB = Integer('0x' + b[5, 2]).to_f / 255.0
    rC = rB * amount + rA * (1.0 - amount)
    gC = gB * amount + gA * (1.0 - amount)
    bC = bB * amount + bA * (1.0 - amount)
    result = sprintf('#%02x%02x%02x', (rC * 255.0).to_i, (gC * 255.0).to_i, (bC * 255.0).to_i)
    return result
end

puts '<html>'
puts '<head>'
puts '<title>Tango Palette</title>'
puts '</head>'
puts '<body>'
[100, 50, 25].each do |p|
    puts "<h3>#{p}%</h3>"   
    puts "<table style='font-family: monospace;'>"
    lk_Palette.each do |ls_Name|
        print "<tr><td>#{ls_Name.keys.first}</td><td style='padding: 8px;'>"
        ls_Name.values.first.each do |ls_Color|
            color = mix(
                '#ffffff', 
                '#' + ls_Color, 
                p.to_f / 100.0)
            print "<span style='padding: 8px; background-color: #{color}'>#{color}</span>"
        end
        puts "</td></tr>"
    end
    puts '</table>'
end
puts '</body>'
puts '</html>'
