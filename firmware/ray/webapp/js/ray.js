// CH sliders /////////////////////////////////////////////////////////////////
var slider_timer;
var ch_slider = [-1, -1, -1];
var ignore_ch_update = false;
var all_slider = -1;
var MAX_SLIDERS = 4;

function check_setled_updates()
{
    for (i=0; i<MAX_SLIDERS; i++)
      if (ch_slider[i] >= 0){
          if (!ignore_ch_update) $.ajax({url:"/setLed?ch="+ i +"&val="+ch_slider[i]});
          ch_slider[i] = -1;
      }
      ignore_ch_update = false;
      if (all_slider >= 0) {
        $.ajax({url:"/setLed?val="+all_slider,
                dataType: "json",
                jsonp: false,
                success: update_ch_sliders});
        all_slider = -1;
      }
}

function start_slider_timer() {
    slider_timer = setInterval(check_setled_updates, 100);
}


function update_ch_sliders(data) {

  for (i = 0 ; i < MAX_SLIDERS; i++) {
    // we are updating the slider, but we don't want that to generate
    // another HTTP request to the server
    ignore_ch_update = true;
     $("#ch"+i+"_slider").slider("value", data["ch"+i]);

  }
}

function set_ch_sliders(data) {
    for (i = 0 ; i < MAX_SLIDERS; i++) {
      $("#ch"+i+"_slider").slider(
        { min: 0,
          max: 100,
          value: data["ch"+i] ,
          change: (function(idx){
                    return function( event, ui ) {
                              ch_slider[idx] = ui.value;
                            }
                  })(i)
        });
    }

    $("#all_slider").slider(
      { min: 0,
        max: 100,
        value: data["all"] ,
        change: (function( event, ui ) { all_slider = ui.value; })
      }
    );

    start_slider_timer();
}

function load_sliders() {
    $.ajax({
      url: "/getLeds",
      dataType: "json",
      jsonp: false,
      success: set_ch_sliders
    });
}

// MAIN ///////////////////////////////////////////////////////////////////////

$(function() {
  load_config();
  check_for_updates('ray');
  load_sliders();
});
