require('pin_mapping')
PWM_FREQ=1000
local STARTUP_DUTY=1000

local TIMEOUT_DUTY=1000
local TIMEOUT_MS=0

local GAMMA_FACTOR=1.5

local DIMMER_GPIOS = {DIMMER1_PIN, DIMMER2_PIN, DIMMER3_PIN}
local DIMMER_VALS  = {STARTUP_DUTY, STARTUP_DUTY, STARTUP_DUTY}
local MAX_DIMMERS=3

function init_dimmers()
    function set_all_dimmers_to(val)

        for i=1,MAX_DIMMERS do
            pwm.setup(DIMMER_GPIOS[i], PWM_FREQ, val)
            pwm.start(DIMMER_GPIOS[i])
        end
    end
    set_all_dimmers_to(STARTUP_DUTY)

    -- if we don't get a value in TIMEOUT_MS we set
    -- dimmers to TIMEOUT_DUTY
    if TIMEOUT_MS ~= 0 then
        tmr.alarm(5, TIMEOUT_MS, 0,
            function()
                set_all_dimmers_to(TIMEOUT_DUTY)
            end
         )
     end

end

function gamma(x)
    return math.pow(x/PWM_FREQ, GAMMA_FACTOR)*PWM_FREQ +0.5
end

function set_dimmer(dimmer_n, dutty)
    tmr.stop(5)
    pwm_val = gamma(dutty)
    pwm_pin = DIMMER_GPIOS[dimmer_n]
    pwm.setduty(pwm_pin, pwm_val)
    DIMMER_VALS[dimmer_n] = dutty
end

function get_dimmer(dimmer_n)
    return DIMMER_VALS[dimmer_n]
end

init_dimmers()
