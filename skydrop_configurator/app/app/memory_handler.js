"use strict";

app.service("memory", ["$http", "$q", function($http, $q){
    //Variabiles
    this.ee_buffer = false;
    this.ee_map = false;
    this.ee_desc = false;
    this.build_number = false;
    this.newest_build = false;
    this.fw_path = false;
    this.macros = false; 
    this.layouts = false;
    this.data_holder = false;
    this.data_loading = false;
    this.data_loading_notify = [];
    this.data_loaded = false;
   
    //Methods
    
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
    
    //load configuration data from stream
    this.load_data = function(data)
    {
     	this.init_step_1(data, this);     
     	this.data_loaded = true;
    };    

    //load json data from url and then call cb
    this.load_json = function(url_path, cb)
    {
        var callback = cb;
        var service = this;
        var error_cb = function() {
        	alert("Unable to load description files for build " + service.build_number + "!\nAre you using unofficial build?\nIf you think that this is an error please contact us at info@skybean.eu.");
        	service.load_bin("UPDATE.EE", service.init_step_1);
        };
    
        console.log("loading json resource %s", url_path);

        $http.get(url_path)     
        .success(function(data) {
        	//console.log("sucess");
            //console.log(data);
            callback(data, service);            
        })
        .error(function(xhr, status, error) {
            console.warn("error", xhr, status, error);
            if (error_cb != undefined)
            	error_cb();
            service.reject_all();
      });
    };

    this.reject_all = function(){
    	for (var i = 0; i < this.data_loading_notify.length; i++)
    		this.data_loading_notify[i].reject();
    };

    this.notify_all = function(data){
		console.log("objects to notify", this.data_loading_notify.length);
    	for (var i = 0; i < this.data_loading_notify.length; i++)
    		this.data_loading_notify[i].notify(data);
    };
    
    this.remove_notify = function(deferred)
    {
    	for (var i = 0; i < this.data_loading_notify.length; i++)
    	{
    		if (this.data_loading_notify[i] === deferred)
			{
    			this.data_loading_notify.splice(i);
    			return;			
			}
    	}
    };
    
    this.getIndex = function(key)
    {
        return this.ee_map[key][0];
    };       
    
    this.getLength = function(key)
    {
        return this.ee_map[key][1];
    };    
    
    this.getType = function(key)
    {
        return this.ee_map[key][2];
    };
    
    this.getTypeSize = function(type)
    {
    	var sizes = {
    			"int8_t": 1,
    			"uint8_t": 1,
    			"int16_t": 2,
    			"uint16_t": 2,
    			"int32_t": 4,
    			"uint32_t": 4,
    			"float": 4,
    			"char": 1
    		};
    	
    	return sizes[type];
    };
    
    //get value form binary data (ee_bufer)
    this.getValue = function(key)
    {
        var mem_index = this.getIndex(key);
        var var_type = this.getType(key);
        var var_len = this.getLength(key);
        var var_size = this.getTypeSize(var_type);
        var arr_len = var_len / var_size;
        
//        console.log("arr_len", arr_len);
        
        var val = [];
        for (var i = 0; i < arr_len; i++)
        {
	        switch(var_type)
	        {
	            case('uint8_t'):
	                val.push(get_uint8(this.ee_buffer, mem_index));
	            break;
	            
	            case('int8_t'):
	            	val.push(get_int8(this.ee_buffer, mem_index));
	            break;
	
	            case('uint16_t'):
	            	val.push(get_uint16(this.ee_buffer, mem_index));
	            break;
	                
	            case('int16_t'):
	            	val.push(get_int16(this.ee_buffer, mem_index));
	            break;
	
	            case('uint32_t'):
	            	val.push(get_uint32(this.ee_buffer, mem_index));
	            break;
	
	            case('float'):
	            	val.push(get_float(this.ee_buffer, mem_index));
	            break;
	            
	            case('char'):
	            	val.push(get_char(this.ee_buffer, mem_index));            
	            break;
	        }
	        
	        mem_index += var_size;
	    }
        
        if (val.length == 1)
        	return val[0];
        else
    	{
    		if (var_type == "char")
    			return val.join("")
    		else
    			return val;
    	}
    };

    //set value to binary data (ee_buffer)
    this.setValue = function(key, value)
    {
        var mem_index = this.getIndex(key);
        var var_type = this.getType(key);
        var var_size = this.getTypeSize(var_type);        
        
        console.log("set", value, "to", key, "type", var_type, "index", mem_index);
        console.log(this);
        
        if (!Array.isArray(value))
        	value = [value];
        	
        for (var v in value)
        {
        	mem_index += var_size;
        	
	        switch(var_type)
	        {
	            case('uint8_t'):
	                this.ee_buffer = set_uint8(this.ee_buffer, mem_index, v);
	            break;
	            
	            case('int8_t'):
	                this.ee_buffer = set_int8(this.ee_buffer, mem_index, v);
	            break;
	            
	            case('uint16_t'):
	                this.ee_buffer = set_uint16(this.ee_buffer, mem_index, v);
	            break;
	                
	            case('int16_t'):
	                this.ee_buffer = set_int16(this.ee_buffer, mem_index, v);
	            break;
	            
	            case('uint32_t'):
	                this.ee_buffer = set_uint32(this.ee_buffer, mem_index, v);
	            break;
	
	            case('float'):
	                this.ee_buffer = set_float(this.ee_buffer, mem_index, v);
	            break;                
	            
	            case('char'):
	            	var size = this.data_holder[key].size;
	                this.ee_buffer = set_char(this.ee_buffer, mem_index, v, size);
	            break;               
	        }
        }
    };  
    
    this.listVariabiles = function() 
    {
        var keys;
        
        keys = new Array();
        for (var key in this.ee_map)
            keys.push(key);
            
        return keys;
    };

    this.getDesc = function(k)
    {
        if (k in this.ee_desc.absolute)
            return this.ee_desc.absolute[k];
        else
        {
            for (var expr in this.ee_desc.regexp)
            {
                var re = new RegExp(expr);
            
                if(re.test(k))
                {
                    return this.ee_desc.regexp[expr];
                }
            }
        }
            
        return [];
    };
    
    this.getBlob = function()
    {
        for (var index in this.data_holder)
        {
        	var item = this.data_holder[index];
            var value = this.logic_to_bin(item);
            
            this.setValue(item.pname, value);
        }
        
        return this.ee_buffer; 
    };

    this.getAllValues_async = function()
    {
        var vars = this.listVariabiles();
        var items = {};
    
        for (var i in vars)
        {
            var k = vars[i];
            
            var arr = {
                "pname": k, 
                "value": this.getValue(k),
                "type": this.getType(k),
            };
            
            arr = angular.extend(arr, this.getDesc(k));
          
            arr["value"] = this.bin_to_logic(arr["value"], k, arr);
            
            items[k] = (arr);
        }
        
        return items;
    };


    this.getAllValues = function(deferred)
    {
    	if (typeof deferred !== 'undefined')
    		if (!(deferred in this.data_loading_notify))
    		{
    			this.data_loading_notify.push(deferred);
    		}
    	
    	if (this.data_holder == false)
        {
        	if (this.data_loading == false)
    		{
        		this.data_loading = true;
        		this.load_bin("UPDATE.EE", this.init_step_1);
    		}
        }
        else
        {
            deferred.notify(this.data_holder);
        }
    };    
    
    //Constructor & init
    this.init_step_1 = function(data, service)    
    {
        service.ee_buffer = new Uint8Array(data);
        service.build_number = get_uint32(service.ee_buffer, 0);
        console.log("actual build_number is", service.build_number);        
        if (service.newest_build === false)
        	service.newest_build = service.build_number;
        
        //load ee_map
        service.fw_path = "fw/" + zero_pad(8, service.build_number) + "/";
        service.load_json(service.fw_path + "ee_map.json", service.init_step_2);
    };

    this.init_step_2 = function(data, service)    
    {
        service.ee_map = data.map;
        service.macros = service.solveMacros(data.macros);
        service.layouts = data.layouts;
        
        //load parameters description
        service.load_json("res/desc.json", service.init_step_3);
    };

    this.init_step_3 = function(data, service)    
    {
        service.ee_desc = data;
        
        service.data_holder = service.getAllValues_async();
        
        //console.log(">FRESH>", service.data_holder);
      	service.notify_all(service.data_holder);
    };
    
    
    this.solveMacros = function(macros)
    {
        var res = new Array();
        
        for (var key in macros)
        {
            var val = macros[key];
            var new_val = false;
            
            if (val.substr(0,2).toLowerCase() == "0b")
                new_val = parseInt(val.substr(2,8).toLowerCase(), 2);

            if (val.substr(0,2).toLowerCase() == "0x")
                new_val = parseInt(val.substr(2,8).toLowerCase(), 16);
            
            if (!isNaN(val) && new_val === false)
                new_val = parseInt(val, 10);
            
            if (new_val !== false)
                res[key] = new_val;
            //else
            //    console.log(key + " = " + val)
        }
        
        return res;
    };
    
    this.value_from_macro = function(macro_name)
    {
        if (Number.isInteger(macro_name))
        	return macro_name;
    	
        if (macro_name in this.macros)
        {
            return this.macros[macro_name];
        }
        else
        {
            console.log("Unknown macro name " + macro_name);
          	return 0;
        }
    };
    
    this.bin_to_logic = function(value_in, key, arr)
    {
		var value = {};  

    	switch (arr["mode"])
    	{
    		case("flags"):
    			value.flags = {};
                for (var i=0; i < arr["flags"].length; i++)
                   	value.flags[arr["flags"][i][0]] = (value_in & this.value_from_macro(arr["flags"][i][0])) ? true : false;
			break;
    		
    		case("select"):
                for (var i=0; i < arr["options"].length; i++)
                    if (value_in == this.value_from_macro(arr["options"][i][0]))
                    	value.option = arr["options"][i][0];
			break;
    		
    		case("altimeter"):
    			var opt_mask = this.value_from_macro(arr["options_mask"]);
    			var alt_mask = this.value_from_macro(arr["altimeters_mask"]);
    			
    			var opt = opt_mask & value_in;
    			value.mode = {};
    			value.altimeter = {};
    			
	            for (var i=0; i < arr["options"].length; i++)
	                if (opt == this.value_from_macro(arr["options"][i][0]))
	                	value.mode.option = arr["options"][i][0]; 
	            
	            var alt = alt_mask & value_in;
                for (var i=0; i < arr["altimeters"].length; i++)
                    if (alt == this.value_from_macro(arr["altimeters"][i][0]))
                    	value.altimeter.option = arr["altimeters"][i][0];	            
	            
                value.flags = {};
                for (var i=0; i < arr["flags"].length; i++)
                   	value.flags[arr["flags"][i][0]] = (value_in & this.value_from_macro(arr["flags"][i][0])) ? true : false;
			break;
    		
    		case("doubleselect"):
    			var mask1 = this.value_from_macro(arr["mask1"]);
				var mask2 = this.value_from_macro(arr["mask2"]);
    			
                value.select1 = {};
                value.select2 = {};
                for (var i=0; i < arr["options1"].length; i++)
                    if ((value_in & mask1) == this.value_from_macro(arr["options1"][i][0]))
                    	value.select1.option = arr["options1"][i][0];
    		
 	    		for (var i=0; i < arr["options2"].length; i++)
	                if ((value_in & mask2) == this.value_from_macro(arr["options2"][i][0]))
	                	value.select2.option = arr["options2"][i][0];
    		
    		break;
    		
    		default:
    			value = value_in;
    	}
    	
        return value;
    };
    
    this.logic_to_bin = function(arr)
    {

		console.log(arr);
		
    	switch (arr["mode"])
    	{
    		case("flags"):
    			var ret = 0;
    			for (var key in arr.value.flags)
    				if (arr.value.flags[key])
    					ret |= this.value_from_macro(key);
    			
    			return ret;
			break;
    		
    		case("select"):
                return this.value_from_macro(arr.value.option);
			break;
    		
    		case("altimeter"):
    			var opt_mask = this.value_from_macro(arr["options_mask"]);
    			var alt_mask = this.value_from_macro(arr["altimeters_mask"]);
    			
    			var ret = 0;;
    			
    			ret |= this.value_from_macro(arr.value.mode.option) & opt_mask;
    			ret |= this.value_from_macro(arr.value.altimeter.option) & alt_mask;
          
    			for (var key in arr.value.flags)
    				if (arr.value.flags[key])
    					ret |= this.value_from_macro(key);
	            
    			return ret;
			break;
    		
    		case("doubleselect"):
    			var mask1 = this.value_from_macro(arr["mask1"]);
				var mask2 = this.value_from_macro(arr["mask2"]);
    			
				var ret = 0;
	   			ret |= this.value_from_macro(arr.value.select1.option) & mask1;
    			ret |= this.value_from_macro(arr.value.select2.option) & mask2;
				
				return ret;
    		break;
    		
    		default:
    			return arr.value;
    	}

    	console.log("Unhandled", arr);
    	1/0;
    	
    	return 0;
    };    
    
    this.restore_default = function()
    {
    	this.load_bin(this.fw_path + "UPDATE.EE", this.init_step_1);
    };
    
    this.is_old_version = function()
    {
    	return this.build_number != this.newest_build;    	
    };
    
    this.upgrade = function()
    {
    	var old_values = {};
    	
    	//store old values
    	for (var key in this.data_holder)
    	{
    		var line = this.data_holder[key];
    		if (line.pname != "cfg_build_number")
    			old_values[line.pname] = line.value;
    	}
        
        //save notify list
        var saved_notify = this.data_loading_notify;
        
        //replace notify list with local function 
        var deferred = $q.defer();
        this.data_loading_notify = [deferred];
        
    	//load newest fw
        this.load_bin("UPDATE.EE", this.init_step_1);
        
        var service = this;
        
        deferred.promise.then(undefined, undefined, function (){
        	//restore values
        	for (var key in old_values)
        	{
        		if (key in service.data_holder)
        			service.data_holder[key].value = old_values[key];
        	}
        	
        	//restore notify list
        	service.data_loading_notify = saved_notify;
        	service.notify_all(service.data_holder);
        });    	
    };

    this.get_crc = function(data)
    {
    	function calc_crc(sum, key, data)
    	{
    	    for (var i = 0; i < 8; i++)
    	    {
    	        if ((data & 0x01)^(sum & 0x01))
    	        {
    	            sum = (sum >> 1);
    	            sum = (sum ^ key);
    	        }
    	        else
    	        {
    	            sum = (sum >> 1);
    	        }
    	        data = (data >> 1);
    	    }
    	        
    	    return sum;
    	}
    	
    	var key = 0x9B;
    	var crc = 0;
    	
    	for (var i = 0; i < data.length; i++)
    		crc = calc_crc(crc, key, data[i]);
    	
    	return crc;
    };
    
    this.pack_fw = function(data, service)
    {
    	 var ee_bin = service.getBlob();
    	 var fw_bin = new Uint8Array(data);
    	 
    	 var ee_size = ee_bin.length;
    	 var fw_size = fw_bin.length;
    	 var pack_size = 18 + ee_size + fw_size;
    	 
    	 var ee_crc = service.get_crc(ee_bin);
    	 var fw_crc = service.get_crc(fw_bin);
    	 
    	 var pack = new Uint8Array(pack_size);
    	 //header
    	 pack = set_uint32(pack, 0, service.build_number);
    	 pack = set_uint32(pack, 4, pack_size);
    	 pack = set_uint32(pack, 8, ee_size);
    	 pack = set_uint8(pack, 12, ee_crc);
    	 pack = set_uint32(pack, 13, fw_size);
    	 pack = set_uint8(pack, 17, fw_crc);
    	 //eeprom
    	 for (var i = 0; i < ee_size; i++)
    		 pack[i + 18] = ee_bin[i];
    	 //firmware
    	 for (var i = 0; i < fw_size; i++)
    		 pack[i + 18 + ee_size] = fw_bin[i];
    	 
    	 service.fw_package_defered.resolve(pack);
    };
    
    this.get_fw_pack = function()
    {
        var deferred = $q.defer();
        this.fw_package_defered = deferred;
    	
    	this.load_bin(this.fw_path + "UPDATE.FW", this.pack_fw);
    	
        return deferred.promise;	
    };
    
    this.get_widgets = function()
    {
    	return this.ee_desc.widgets;
    };
    
}]);

