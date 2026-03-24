        @echo *** Please prepare the Nyxel Helmet to be programmed ***
        @pause
:Loop

        @echo *** Start programming Nyxel Helmet! ***
        @nrfjprog --recover -f NRF52 
        @nrfjprog --eraseall -f NRF52 
        @nrfjprog --program Helmet\Nyx_AllInOne.hex --verify -f NRF52

        @echo *** To reset Helmet ***
        @nrfjprog --reset -f NRF52
        @pause

:end
        @goto loop