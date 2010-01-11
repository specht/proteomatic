#!/usr/bin/ruby

require 'yaml'

lk_Files = Dir['*.svg']

lk_Files.each do |ls_Filename|
    system("inkscape --file=#{ls_Filename} --export-png=#{ls_Filename.sub('.svg', '.png')} --export-width=48")
    File::delete(ls_Filename)
end
