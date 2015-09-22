local cfg={}
wifi.sta.config(STA_AP, STA_PASS)
cfg.ssid = AP_NAME
cfg.pwd = AP_PASS
wifi.ap.config(cfg)
wifi.setmode(wifi.STATIONAP)

