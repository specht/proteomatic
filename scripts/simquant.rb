require 'include/proteomatic'
require 'include/externaltools'
require 'include/fasta'
require 'include/formats'
require 'include/misc'
require 'yaml'
require 'fileutils'

class SimQuant < ProteomaticScript
	def run()
		lk_Peptides = @param[:peptides].split(%r{[,;\s/]+})
		lk_Peptides.reject! { |x| x.strip.empty? }
		
		@input[:peptidesFile].each do |ls_Path|
			next unless fileMatchesFormat(ls_Path, 'txt')
			ls_File = File::read(ls_Path)
			ls_File.each do |ls_Line|
				ls_Peptide = ls_Line.strip
				next if ls_Peptide.empty?
				lk_Peptides.push(ls_Peptide)
			end
		end
		
		lk_Peptides.uniq!
		if lk_Peptides.empty?
			puts 'Error: no peptides have been specified.'
			exit 1
		end
		
		ls_TempPath = tempFilename('simquant')
		ls_YamlPath = File::join(ls_TempPath, 'out.yaml')
		ls_SvgPath = File::join(ls_TempPath, 'svg')
		FileUtils::mkpath(ls_TempPath)
		FileUtils::mkpath(ls_SvgPath)
		
		ls_Command = "#{ExternalTools::binaryPath('simquant.simquant')} --textOutput no --yamlOutput yes --yamlOutputTarget #{ls_YamlPath} --svgOutPath #{ls_SvgPath} --files #{@input[:spectra].join(' ')} --peptides #{lk_Peptides.join(' ')}"
		puts 'There was an error while executing simquant.' unless system(ls_Command)
		
		lk_Results = YAML::load_file(ls_YamlPath)
		
		if @output[:xhtmlReport]
			File.open(@output[:xhtmlReport], 'w') do |lk_Out|
				lk_Out.puts "<?xml version='1.0' encoding='utf-8' ?>"
				lk_Out.puts "<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.1//EN' 'http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd'>"
				lk_Out.puts "<html xmlns='http://www.w3.org/1999/xhtml' xml:lang='de'>"
				lk_Out.puts '<head>'
				lk_Out.puts '<title>SIM Quantitation Results</title>'
				lk_Out.puts '<style type=\'text/css\'>'
				lk_Out.puts 'body {font-family: Verdana; font-size: 10pt;}'
				lk_Out.puts 'h1 {font-size: 14pt;}'
				lk_Out.puts 'h2 {font-size: 12pt; border-top: 1px solid #888; border-bottom: 1px solid #888; padding-top: 0.2em; padding-bottom: 0.2em; background-color: #e8e8e8; }'
				lk_Out.puts 'h3 {font-size: 10pt; }'
				lk_Out.puts 'h4 {font-size: 10pt; font-weight: normal;}'
				lk_Out.puts 'ul {padding-left: 0;}'
				lk_Out.puts 'ol {padding-left: 0;}'
				lk_Out.puts 'li {margin-left: 2em;}'
				lk_Out.puts '.default { }'
				lk_Out.puts '.nonDefault { background-color: #ada;}'
				lk_Out.puts 'table {border-collapse: collapse;} '
				lk_Out.puts 'table tr {text-align: left; font-size: 10pt;}'
				lk_Out.puts 'table th, table td {vertical-align: top; border: 1px solid #888; padding: 0.2em;}'
				lk_Out.puts 'table tr.sub th, table tr.sub td {vertical-align: top; border: 1px dashed #888; padding: 0.2em;}'
				lk_Out.puts 'table th {font-weight: bold;}'
				lk_Out.puts '.gpf-confirm { background-color: #aed16f; }'
				lk_Out.puts '.toggle { text-decoration: underline; color: #aaa; }'
				lk_Out.puts '.toggle:hover { cursor: pointer; color: #000; }'
				lk_Out.puts '</style>'
				lk_Out.puts '</head>'
				lk_Out.puts '<body>'
				lk_Out.puts "<table style='min-width: 820px;'>"
				lk_Peptides = lk_Results['results'].keys.sort
				lk_Peptides.each do |ls_Peptide|
					lk_Out.puts "<tr><td colspan='5' style='border: none; padding-top: 1em; padding-bottom: 1em;'><b>#{ls_Peptide}</b></td></tr>"
					lk_Out.puts "<tr><th>Spot</th><th>Charge</th><th>Ratio</th><th>Certainty</th><th>Scans</th></tr>"
					lk_PeptideResults = lk_Results['results'][ls_Peptide]
					lk_PeptideResults.sort! do |a, b|
						(a['file'] == b['file']) ? 
							a['charge'].to_i <=> b['charge'].to_i :
							String::natcmp(a['file'], b['file'])
					end
					lk_PeptideResults.each do |lk_Row|
						lk_SubRows = lk_Row['scans']
						lk_SubRows.sort! { |a, b| b['score'] <=> a['score'] }
						lk_Out.puts "<tr><td>#{lk_Row['file']}</td><td>#{lk_Row['charge']}</td><td>#{sprintf("%1.2f", lk_Row['ratio'].to_f)}</td><td>#{sprintf("%1.2f", lk_Row['certainty'].to_f * 100.0)} %</td><td><span class='toggle'>#{lk_SubRows.size} scans</span></td></tr>"
						lk_SubRows.each do |lk_SubRow|
							ls_Svg = File::read(File::join(ls_SvgPath, lk_SubRow['svg'] + '.svg'))
							ls_Svg.sub!(/<\?xml.+\?>/, '')
							ls_Svg.sub!(/<svg width=\".+\" height=\".+\"/, '<svg ')
							lk_Out.puts "<tr class='sub'><td>#{lk_Row['file']}</td><td>#{lk_Row['charge']}</td><td>#{sprintf("%1.2f", lk_SubRow['ratio'].to_f)}</td><td>#{sprintf("%1.2f", lk_SubRow['certainty'].to_f * 100.0)} %</td><td></td></tr>"
							lk_Out.puts "<tr class='sub'><td colspan='5'>"
							lk_Out.puts ls_Svg
							lk_Out.puts "<div>#{lk_Row['file']} ##{lk_SubRow['id']} @ #{lk_SubRow['retentionTime'].to_f} minutes</div>"
							lk_Out.puts "</td></tr>"
						end
					end
				end
				lk_Out.puts '</table>'
				lk_Out.puts '</body>'
				lk_Out.puts '</html>'
			end
		end
		puts "AAAAAAARGH! CLANK CLANK YOU'RE DEAD."
		exit 1
	end
end

lk_Object = SimQuant.new
