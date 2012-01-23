-- SocketFilter.lua

--
-- Filter für den SocketServer - wird pro aktiven Client aufgerufen
-- 
-- Globales Array "arg" enthält die Daten vom Auswertermodul
--
-- Zusätzliche Werte im Array:
--
-- client_authenticated: 	0/1  (1=angemeldet)
-- client_ip:			IP-Adresse des Clients
-- client_loginname: 		Anmeldename
-- client_type:			fms32, crusader, monitord
--
-- Rückgabewert: 0 = an Client senden, 1= nicht an Client senden, alle anderen = an Client senden
--

local toShowFMS = {"11111111", "22222222"} ;
local toShowPOCSAG = {"1111111", "2222222"} ;
local toShowZVEI = {"99000", "99001"} ;

function myfilterFMS32()
	for index,testwert in pairs(toShowFMS) do 
		if (testwert==arg["ric"]) then
			return 0
		end
	end
	return 1 ;
end

function myfilterMONITORD()
	-- ZVEI
	if(arg["typ"] == "zvei") then
		for index,testwert in pairs(toShowZVEI) do 
			if(testwert == arg["zvei"]) then
				return 0
			end
		end
	end

	-- FMS
	if(arg["typ"] == "fms") then
		for index,testwert in pairs(toShowFMS) do 
			if(testwert == arg["fmskennung"]) then
				return 0
			end
		end
	end

	-- POCSAG
	if(arg["typ"] == "pocsag") then
		for index,testwert in pairs(toShowPOCSAG) do 
			if(testwert == arg["ric"]) then
				return 0
			end
		end
	end
	
	-- default: anzeige unterdruecken
	return 1;
end
	
function filter() 

	local dummyValue=1 ;

	-- DEBUG-Info: Alles ausgeben
	for index,testwert in pairs(arg) do 
		print(index)
		print(testwert)
	end	
	
	-- wird für jedes Telegramm (pocsag, fms, zvei) aufgerufen
	if (arg["client_type"]=="fms32") then
		return 0; -- delete this line to enable and uncomment the next one!
		-- return myfilterFMS32() ;
	end
	
	if (arg["client_type"]=="crusader") then
		-- nix
		dummyValue=2 ;
	end

	if (arg["client_type"]=="monitord") then
		return 0; -- delete this line to enable and uncomment the next one!
		-- return myfilterMONITORD() ;
	end	

	-- default: alles anzeigen; ändern auf "1" um nichts anzuzeigen!
	return 0;

end
