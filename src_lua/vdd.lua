-- this is for a 15k + 1k voltage divider
function get_vdd_in()
 return adc.read(0) / 1024 * (15.873)
end
