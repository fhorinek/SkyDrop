"use strict";

//chart cfg
Chart.defaults.global.responsive = true;
Chart.defaults.global.animation = false;

Chart.defaults.global.scaleStartValue = 0;
Chart.defaults.global.scaleStepWidth = 200;
Chart.defaults.global.scaleSteps = 10;
Chart.defaults.global.scaleOverride = true;

Chart.defaults.Line.bezierCurve = false;

app.controller("audioProfile", ['$scope', '$http', 'memory', "ChartJs", "$q", function ($scope, $http, memory, chartjs, $q) {
	var deferred = $q.defer();

	deferred.promise.then(undefined, undefined, function (data){
		$scope.list = data;
		$scope.refresh();
	});	
	
	memory.getAllValues(deferred);
	
	$scope.list = {};
	
	$scope.$on('$destroy', function() {
		memory.remove_notify(deferred);
	});
	
	var audio = false;

	$scope.demo_val = 0.0;

    try 
    {
        audio = new Audio();
    }
    catch(err) 
    {
        audio = false;
    }            

    $scope.refresh = function()
    {
    	var freq = [];
    	var pause = [];
    	var length = [];
    	
    	for (var i=0; i < 41; i++)
    	{
    		freq.push($scope.list["cfg_audio_profile_freq_" + i].value);
    		pause.push($scope.list["cfg_audio_profile_pause_" + i].value);
    		length.push($scope.list["cfg_audio_profile_length_" + i].value);
    	}
    
    	$scope.data = [freq, pause, length];  
    };
	
	$scope.labels = [];
	
	for (var i = -10 ; i <= 10; i += 0.5)
		$scope.labels.push(i + " m/s");
	
	$scope.series = ['Frequency [Hz]', 'Pause [ms]', 'Length [ms]'];
		
	$scope.data = [];
	$scope.point_drag = false;
	$scope.point_series = false;
	$scope.point_index = false;
	$scope.c_freq = "N/A";
	$scope.c_paus = "N/A";
	$scope.c_leng = "N/A";
	$scope.c_state = "Idle";
	
	$scope.onHover = function (points, evt, chart) 
	{

		if ($scope.point_drag)
		{
			$scope.point_index = points[0].index;
			
			var name = ["cfg_audio_profile_freq_", "cfg_audio_profile_pause_", "cfg_audio_profile_length_"][$scope.point_series] + $scope.point_index;
			
			var val = $scope.list[name].value;
			var y = evt.layerY;

			
			val = in_range(chart.scale.calculateVal(y), 0, 2000);
			
			//store to data holder
			$scope.list[name].value = val;
			//update graph
			$scope.data[$scope.point_series][$scope.point_index] = val;    
		}
	};	
	
	$scope.onUp = function(points, evt)
	{
		$scope.point_drag = false;
	};

	$scope.onDown = function(points, evt)
	{
//	    console.log("Down", points, evt);
	    var x = evt.layerX;
	    var y = evt.layerY;
	    
	    var dist = false;
	    
	    for (var i in points)
    	{
	    	var point = points[i];
	    	
	    	var p_dist = Math.pow(point.x - x, 2) + Math.pow(point.y - y, 2); 
	    	
	    	if (p_dist < dist || !dist)
	    	{
	    		dist = p_dist;
	    		$scope.point_series = $scope.series.indexOf(point.datasetLabel);
	    		$scope.point_index = point.index;
	    		$scope.point_drag = true;
	    	}
    	
    	}
	};
	
	$scope.demoFormat = function(value)
	{
	    if (value < 0)
	        return "Sink " + value + " m/s";
	    else
    	    return "Lift " + value + " m/s";
	};
	
	$scope.demo_dec = function()
	{
	    if ($scope.demo_val > -10.0)
    	    $scope.demo_val -= 0.1;
    	$scope.onDemoSlide($scope.demo_val);
   	};

	$scope.demo_inc = function()
	{
	    if ($scope.demo_val < 10.0)
    	    $scope.demo_val += 0.1;
    	$scope.onDemoSlide($scope.demo_val);
	};
	
	$scope.onDemoSlide = function(value)
	{
        function get_near(vario, src)
        {
	        vario = vario * 2; //1 point for 50cm
	        var findex = Math.floor(vario) +  20;
	        var m = vario - Math.floor(vario);

	        var index = findex;

	        if (findex > 39)
	        {
		        index = 39;
		        m = 1.0;
	        }

	        if (findex < 0)
	        {
		        index = 0;
		        m = 0.0;
	        }

	        var start = src[index];

	        start = start + (src[index + 1] - start) * m;

	        return start;
        };	
        
        var MS_TO_TICKS = 1;
        
	    var ivario = value * 100; //in cm now

	    var lift = $scope.list["cfg_audio_profile_lift"].value;
	    var sink = $scope.list["cfg_audio_profile_sink"].value;
	    var weak = $scope.list["cfg_audio_profile_weak"].value;	    
	    
	    var prebeep_offset = $scope.list["cfg_audio_profile_prebeep_offset"].value;	 
	    var prebeep_length = $scope.list["cfg_audio_profile_prebeep_length"].value;	 
	    
        var sink_mode = $scope.list["cfg_audio_profile_flags"].value.select1.option;
        var weak_mode = $scope.list["cfg_audio_profile_flags"].value.select2.option;
        var fluid = $scope.list["cfg_audio_profile_flags"].value.select3.option	== "AUDIO_FLUID";

	    if (ivario >= lift ||
		    (ivario >= lift - weak && (weak_mode != "AUDIO_WEAK_OFF")) ||
		     ivario <= sink)
	    {
		    //get frequency from the table
		    var audio_vario_freq = get_near(ivario / 100.0, $scope.data[0])

		    //if normal lift, weak or sink
		    var audio_vario_length = get_near(ivario / 100.0, $scope.data[2]) * MS_TO_TICKS;
		    var audio_vario_pause = get_near(ivario / 100.0, $scope.data[1]) * MS_TO_TICKS;

		    var audio_vario_prebeep_length = 0;
		    var audio_vario_prebeep_frequency = 0;
		    
            $scope.c_state = "Lift";

		    if (ivario >= lift - weak && ivario < lift && weak_mode != "AUDIO_WEAK_OFF")
		    {
			    switch(weak_mode)
			    {
			        case("AUDIO_WEAK_BEEP"):
			            var audio_vario_prebeep_length = prebeep_length * MS_TO_TICKS;
			            var audio_vario_prebeep_frequency = audio_vario_freq - prebeep_offset;

                        $scope.c_state = "Weak beep";
                    break;
                    
			        case("AUDIO_WEAK_CONT"):
					    audio_vario_length = 0;
					    audio_vario_pause = 0;

					    var lift_start_freq = $scope.list["cfg_audio_profile_weak_high_freq"].value;

					    audio_vario_freq = $scope.list["cfg_audio_profile_weak_low_freq"].value;
					    audio_vario_freq += ((lift_start_freq - $scope.list["cfg_audio_profile_weak_low_freq"].value) * (ivario - ($scope.list["cfg_audio_profile_lift"].value - $scope.list["cfg_audio_profile_weak"].value))) / $scope.list["cfg_audio_profile_weak"].value;			        

                        $scope.c_state = "Weak cont.";
                    break;
                }
		    }

		    if (ivario <= sink)
		    {
		        switch(sink_mode)
		        {
		            case("AUDIO_SINK_BEEP"):
			            //if sink and sink_prebeep -> add prebeep
			            var audio_vario_prebeep_length = prebeep_length * MS_TO_TICKS;
			            var audio_vario_prebeep_frequency = audio_vario_freq + prebeep_offset;

                        $scope.c_state = "Sink beep";

                    break;
                    
		            case("AUDIO_SINK_CONT"):
			            //if sink is continous
			            audio_vario_length = 0;
			            audio_vario_pause = 0;
			            
                        $scope.c_state = "Sink cont.";
                    break;                    
                    
		            case("AUDIO_SINK_OFF"):
                        $scope.c_state = "Sink off";
                        return;
                    break;                         
                }
		    }


            var pb_leng = (audio_vario_prebeep_length) ? Math.round(audio_vario_prebeep_length) + " ms / " : "";
            var pb_freq = (audio_vario_prebeep_frequency) ? Math.round(audio_vario_prebeep_frequency) + " Hz / " : "";

            $scope.c_freq = pb_freq + (Math.round(audio_vario_freq) + " Hz");
            $scope.c_paus = (audio_vario_pause) ? Math.round(audio_vario_pause) + " ms" : "N/A";
            $scope.c_leng = pb_leng + ((audio_vario_length) ? Math.round(audio_vario_length) + " ms" : "N/A");

		    //update audio with new settings
		    $scope.play_beep(audio_vario_freq, audio_vario_length, audio_vario_pause, audio_vario_prebeep_frequency, audio_vario_prebeep_length);

		    return;
	    }

        $scope.c_state = "Idle";
        $scope.c_freq = "N/A";
        $scope.c_paus = "N/A";
        $scope.c_leng = "N/A";
        
        if (!audio.paused)
            audio.pause();        
	};
	
	// freq in Hz
	// len in ms
	// pause in ms
	$scope.play_beep = function(freq, leng, paus, pfreq, pleng)
	{
	    if (audio == false)
	        return;
	        
        if (!audio.paused)
            audio.pause();
	
        console.log("generating demo for", freq, leng, paus, pfreq, pleng);
        var rate = 31250;
        
        var data = [];
        
        // duration in sec
        var duration = 3;
        
        for (var i=0; i< rate * duration; i++) 
        {
            var ms = (i / rate) * 1000;
            
            var total_duration = leng + paus + pleng
            
            //beep
            if (ms % (total_duration) <= (leng + pleng) || (leng == 0 && paus == 0))
            {
                var val;
                
                if (ms % (total_duration) <= pleng)
                    var T = (rate / pfreq)
                else
                    var T = (rate / freq);
            
                if (i % T < T/2)
                    val = 10;
                else
                    val = 0;
                   
                data[i] = val;
            }
            //silent
            else
            {
                data[i] = data[i - 1];
            }
        }
            
        data[i - 1] = Math.random() * 256;
        
        var wave = new RIFFWAVE(); // create the wave file
        
        wave.header.sampleRate = rate;
        wave.Make(data);
        
        audio.src = wave.dataURI; // create the HTML5 audio element
        audio.play(); 
    };
	
    $scope.save_to_file = function()
    {
    	var audio_data = {};
    	
    	audio_data.system = {};
    	audio_data.system["build"] = memory.build_number;
    	audio_data.system["version"] = 0;
    	audio_data.system["mode"] = "array";
    	audio_data.system["min"] = -10;
    	audio_data.system["max"] = 10;
    	audio_data.system["step"] = 0.5;
    	audio_data.system["count"] = 41;
    	
    	audio_data.freq = {};
    	audio_data.length = {};
    	audio_data.pause = {};
    	
    	for (var i=0; i < 41; i++)
    	{
    		var label = ((i - 20) / 2) + " m/s";
    		
    		audio_data.freq[label] = $scope.list["cfg_audio_profile_freq_" + i].value;
    		audio_data.pause[label] = $scope.list["cfg_audio_profile_pause_" + i].value;
    		audio_data.length[label] = $scope.list["cfg_audio_profile_length_" + i].value;
    	}
    	
    	var json_data = JSON.stringify(audio_data);
    	
    	var blob = new Blob([json_data], {type: "text/json"});
    	saveAs(blob, "audioprofile.json");
    }
    
    $scope.load_from_file = function()
    {
    	$scope.file_selector.click();
    }

    $scope.read_file = function(files){
    	var file = files[0];
    	
    	if (files.length != 1 || (file.name.slice(-5) != ".json"))
    	{
    		alert("Please select correct audio profile file (.json).");
    		$scope.file_selector.val(null);
    		return;
    	}
    	
    	var reader = new FileReader();
    	reader.onload = function(e) 
    	{
    	    var contents = e.target.result;
    	    $scope.load_audio_json(contents);
    	    $scope.file_selector.val(null);
    	};
    	  
    	reader.readAsText(file);
    };    
    
    
    
    //load binary (EE or FW) data from url and then call cb
    this.load_bin = function(url_path, cb, error_cb)
    {
        var callback = cb;
        var service = this;
    
        console.log("loading bin resource %s", url_path);

        $http.get(
            url_path,
            {
                responseType: "arraybuffer"
            }
        )     
        .success(function(data) {
            callback(data, service);            
        })
        .error(function(xhr, status, error) {
            console.warn("XHR error", xhr, status, error);
            if (error_cb != undefined)
            	error_cb();
            service.reject_all();
        });
    };    
    
    
    $scope.load_default = function()
    {
        if (!confirm('Restore default audio profile?\nYour changes will be discarted!'))
            return;
    
        $http.get("res/audioprofile.json")     
        .success(function(data) {
            $scope.load_audio_json(data);            
        })
        .error(function(xhr, status, error) {
            console.warn("XHR error", xhr, status, error);
        });
    }    
    
    $scope.load_audio_json = function(data)
    {
        if (data instanceof Object)
            var json_data = data;
        else
        	var json_data = JSON.parse(data);
        	
    	var pass = true;
    	
    	if (json_data.system == undefined)
    		pass = false;
    	if (json_data.system.version == undefined)
    		pass = false;
    	
    	if (pass == false)
		{
    		alert("This file is not valid audio profile!");
    		return;
		}
    	
    	if (json_data.system.version == 0)
    	{
    		var tmp_data = [];
    		
    		try
    		{
	        	for (var i=0; i < 41; i++)
	        	{
	        		var label = ((i - 20) / 2) + " m/s";
	        		
	        		tmp_data["cfg_audio_profile_freq_" + i] = json_data.freq[label];
	        		tmp_data["cfg_audio_profile_pause_" + i] = json_data.pause[label];
	        		tmp_data["cfg_audio_profile_length_" + i] = json_data.length[label];
	        	}    
    		}
    		catch (e)
    		{
    			alert("Unexpected error during load: " + e);
    			pass = false;
    		}
    		
    		if (pass)
    		{
	    		for (var key in tmp_data)
	    			$scope.list[key].value = tmp_data[key];
	    		
	    		alert("Audioprofile loaded sucessfully!");
	    		$scope.refresh();
    		}
    	}
    }
    
    $scope.file_selector = angular.element("#audio-profile-file-selector");
}]);

