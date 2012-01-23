-- dispatcher.lua

--
-- Filter für den Globaldispatcher
-- 
-- Globales Array "arg" enthält die Daten vom Auswertermodul
--
-- Rückgabewert: 0 = an Clients senden, 1= nicht an Clients senden
--

local sperrliste = {"0174784", "1398098"} ;

function filter()
-- wird für jedes Telegramm (pocsag, fms, zvei) aufgerufen
	for index,testwert in pairs(sperrliste) do 
		if (testwert==arg["ric"]) then
			return 1
		end
	end
	
	return 0;
end

