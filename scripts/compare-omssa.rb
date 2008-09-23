require 'include/proteomatic'
require 'include/evaluate-omssa-helper'
require 'include/fastercsv'
require 'include/misc'
require 'set'
require 'yaml'

class CompareOmssa < ProteomaticScript
	def run()
		lk_RunResults = Hash.new
		# compare multiple OMSSA results
		@input[:omssaResults].each do |ls_OmssaResultFile|
			ls_Key = File::basename(ls_OmssaResultFile)
			ls_Key.sub!('omssa-results.csv', '')
			ls_Key = ls_Key[0, ls_Key.size - 1] while ((!ls_Key.empty?) && ('_-. '.include?(ls_Key[ls_Key.size - 1, 1])))
			lk_Result = evaluateFiles([ls_OmssaResultFile], @param[:targetFpr])
			lk_RunResults[ls_Key] = lk_Result
		end
		
		if @output[:htmlReport]
			File.open(@output[:htmlReport], 'w') do |lk_Out|
				lk_RunKeys = lk_RunResults.keys.sort { |x, y| String::natcmp(x, y) }
				
				lk_Out.puts '<html>'
				lk_Out.puts '<head>'
				lk_Out.puts '<title>OMSSA Comparison Report</title>'
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
				lk_Out.puts 'table th, td {vertical-align: top; border: 1px solid #888; padding: 0.2em;}'
				lk_Out.puts 'table th {font-weight: bold;}'
				lk_Out.puts '.gpf-confirm { background-color: #aed16f; }'
				lk_Out.puts '.toggle { cursor: pointer; text-decoration: underline; color: #aaa; }'
				lk_Out.puts '.toggle:hover { color: #000; }'
				lk_Out.puts '</style>'
				lk_Out.puts "<script type='text/javascript'>"
				lk_Out.puts "/*<![CDATA[*/"
				lk_Out.puts "function toggle(as_Name, as_Display) {"
				lk_Out.puts "lk_Elements = document.getElementsByClassName(as_Name);"
				lk_Out.puts "for (var i = 0; i < lk_Elements.length; ++i)"
				lk_Out.puts "lk_Elements[i].style.display = lk_Elements[i].style.display == 'none' ? as_Display : 'none';"
				lk_Out.puts "}"
				lk_Out.puts "/*]]>*/"
				lk_Out.puts "</script>"
				lk_Out.puts '</head>'
				lk_Out.puts '<body>'
				lk_Out.puts '<h1>OMSSA Comparison Report</h1>'
			
				lk_Out.puts '<table>'
				lk_Out.puts "<tr><th rowspan='2'>Protein</th><th colspan='#{lk_RunKeys.size}'>Spectra count</th><th rowspan='2'>std. dev.</th><th colspan='#{lk_RunKeys.size}'>Distinct peptide count</th><th rowspan='2'>std. dev.</th></tr>"
				lk_Out.puts "<tr>#{lk_RunKeys.collect { |x| '<th>' + x + '</th>'}.join('') }#{lk_RunKeys.collect { |x| '<th>' + x + '</th>'}.join('') }</tr>"
				# collect proteins from all runs
				lk_AllProteinsSet = Set.new
				lk_RunKeys.each { |ls_Key| lk_AllProteinsSet.merge(lk_RunResults[ls_Key][:proteins].keys) }
				lk_Proteins = lk_AllProteinsSet.to_a.sort { |x, y| String::natcmp(x, y) }
				lk_Proteins.each do |ls_Protein|
					lk_Out.print "<tr><td>#{ls_Protein}</td>"
					
					lk_Values = Array.new
					ls_SpectraCountString = lk_RunKeys.collect do |ls_Key|
						li_Count = 0
						li_Count = lk_RunResults[ls_Key][:proteins][ls_Protein]['spectraCount'] if lk_RunResults[ls_Key][:proteins].has_key?(ls_Protein)
						lk_Values.push(li_Count)
						"<td>#{li_Count}</td>"
					end.join('')
					lk_Out.print ls_SpectraCountString
					lk_Out.print "<td>#{sprintf("%1.2f", stddev(lk_Values))}</td>"
					
					lk_Values = Array.new
					ls_DistinctPeptidesCountString = lk_RunKeys.collect do |ls_Key|
						li_Count = 0
						li_Count = lk_RunResults[ls_Key][:proteins][ls_Protein]['peptides'].size if lk_RunResults[ls_Key][:proteins].has_key?(ls_Protein)
						lk_Values.push(li_Count)
						"<td>#{li_Count}</td>"
					end.join('')
					lk_Out.print ls_DistinctPeptidesCountString
					lk_Out.print "<td>#{sprintf("%1.2f", stddev(lk_Values))}</td>"
					
					lk_Out.print "</tr>"
					lk_Out.puts
				end
				lk_Out.puts '</table>'
				
				
				lk_Out.puts '</body>'
				lk_Out.puts '</html>'
			end
		end
		if @output[:csvReport]
			File.open(@output[:csvReport], 'w') do |lk_Out|
				lk_RunKeys = lk_RunResults.keys.sort { |x, y| String::natcmp(x, y) }
				
=begin				
				lk_Out.puts "Protein;Spectra count</th><th rowspan='2'>std. dev.</th><th colspan='#{lk_RunKeys.size}'>Distinct peptide count</th><th rowspan='2'>std. dev.</th></tr>"
				lk_Out.puts "<tr>#{lk_RunKeys.collect { |x| '<th>' + x + '</th>'}.join('') }#{lk_RunKeys.collect { |x| '<th>' + x + '</th>'}.join('') }</tr>"
				# collect proteins from all runs
				lk_AllProteinsSet = Set.new
				lk_RunKeys.each { |ls_Key| lk_AllProteinsSet.merge(lk_RunResults[ls_Key][:proteins].keys) }
				lk_Proteins = lk_AllProteinsSet.to_a.sort { |x, y| String::natcmp(x, y) }
				lk_Proteins.each do |ls_Protein|
					lk_Out.print "<tr><td>#{ls_Protein}</td>"
					
					lk_Values = Array.new
					ls_SpectraCountString = lk_RunKeys.collect do |ls_Key|
						li_Count = 0
						li_Count = lk_RunResults[ls_Key][:proteins][ls_Protein]['spectraCount'] if lk_RunResults[ls_Key][:proteins].has_key?(ls_Protein)
						lk_Values.push(li_Count)
						"<td>#{li_Count}</td>"
					end.join('')
					lk_Out.print ls_SpectraCountString
					lk_Out.print "<td>#{sprintf("%1.2f", stddev(lk_Values))}</td>"
					
					lk_Values = Array.new
					ls_DistinctPeptidesCountString = lk_RunKeys.collect do |ls_Key|
						li_Count = 0
						li_Count = lk_RunResults[ls_Key][:proteins][ls_Protein]['peptides'].size if lk_RunResults[ls_Key][:proteins].has_key?(ls_Protein)
						lk_Values.push(li_Count)
						"<td>#{li_Count}</td>"
					end.join('')
					lk_Out.print ls_DistinctPeptidesCountString
					lk_Out.print "<td>#{sprintf("%1.2f", stddev(lk_Values))}</td>"
					
					lk_Out.print "</tr>"
					lk_Out.puts
				end
				lk_Out.puts '</table>'
				
				
				lk_Out.puts '</body>'
				lk_Out.puts '</html>'
=end				
			end
		end
	end
end

lk_Object = CompareOmssa.new
