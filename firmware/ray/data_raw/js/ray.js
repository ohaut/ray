function load_config()
{
  $.ajax({
    url: "/config.json",
    dataType: "json",
    jsonp: false,
    success: function( data ) {
        for (var key in data) {
          $('#main').find('input[name="'+key+'"]').val(data[key]);
        }
        $("#startupModal").modal("hide");
        $("#startupModal").css("display", "none");
    }
  });

  $("#config-form").ajaxForm({url: "/config.json", type:"post",
                              success: function () {
                                alert("config saved, rebooting");
                                location.reload();
                              }});
}

function check_for_updates() {
  function check_latest_version(my_version, my_app_version, commit_id) {
    function parse_csv_and_check(csv) {
        arrayOfLines = csv.match(/[^\r\n]+/g);
        latest_version = "0";
        latest_app_version = "0";
        arrayOfLines.shift(); // remove the description line
        for (var line in arrayOfLines) {
          values = arrayOfLines[line].split(',');
          version = values[0];
          firmware_file = values[1];
          srv_app_version = values[2];
          app_file = values[3];
          commit_id = values[4];
          if (version > latest_version) {
            latest_version = version;
            latest_commit = commit_id;
          }
          if (srv_app_version > latest_app_version ) {
            latest_app_version = srv_app_version;
            latest_commit = commit_id;
          }
        }

        $("span.server_version").html(
          "<a href=\"https://github.com/ohaut/ray/commit/" +
          commit_id +"\" target=\"_blank\">FW:" + latest_version + " HTML:" +
          latest_app_version + "</a>");

        if ((latest_version > my_version) ||
            (latest_app_version > my_app_version))  {
          $("#newer_version").show();
        } else {
          $("#up_to_date").show();
        }


    }
    $.ajax({url: "http://ohaut.org/ray/firmware/master/firmware.csv?r="+
                Math.random(),
            success: parse_csv_and_check});
  }

  $.ajax({url: "/update/status",
          dataType: "json",
          jsonp: false,
          success: function( data ) {
                      for (var key in data)
                        $('#'+key).html(data[key]);
                      check_latest_version(data['firmware_version'], app_version);
                    }});
}

var update_check_timer;
var current_try;
var max_update_check_tries;

function check_update_finished() {

  console.log("Checking update status, try counter: "+ current_try);

  $.ajax({url: "/update/status",
          dataType: "json",
          jsonp: false,
          success: function(data) {
            $("#updateModal").modal("hide");
            clearInterval(update_check_timer);
            console.log("Finished updating firmware");
            location.reload();

          }})

  if (current_try++ > max_update_check_tries)  {
    $("#updateModal").modal("hide");
    clearInterval(update_check_timer);
    location.reload();
  }

  value=(current_try * 100)/max_update_check_tries;
  $('#update_progress').css('width', value+'%').attr('aria-valuenow', value);
}

function start_update_check_timer() {
  update_check_timer = setInterval(check_update_finished, 5000);
  max_update_check_tries = (60*4)/5; /* 4 minutes max */
  current_try = 0;
}

// CH sliders /////////////////////////////////////////////////////////////////
var slider_timer;
var ch_slider = [-1, -1, -1];
var MAX_SLIDERS = 3;
function check_setled_updates()
{
    for (i=0; i<MAX_SLIDERS; i++)
      if (ch_slider[i] >= 0){
          $.ajax({url:"/setLed?ch="+ i +"&val="+ch_slider[i]});
          ch_slider[i] = -1;
      }
}

function start_slider_timer() {
    slider_timer = setInterval(check_setled_updates, 1000);
}

function set_ch_sliders() {
    for (i = 0 ; i < MAX_SLIDERS; i++)
      $("#ch"+i+"_slider").slider({ min: 0,
                                    max: 100,
                                    value: 10,
                                    slide: function( event, ui ) {
                                       ch_slider[i] = ui.value;
                                   }});
    start_slider_timer();
}

// HTML onclick handlers //////////////////////////////////////////////////////
function start_firmware_update() {
  $.ajax({url: "/update/all",
          success: function( data ) {
            $("#updateModal").modal("show");
            start_update_check_timer();
          }})
  return false;
}

// MAIN ///////////////////////////////////////////////////////////////////////

$(function() {
  load_config();
  check_for_updates();
  set_ch_sliders();
});
