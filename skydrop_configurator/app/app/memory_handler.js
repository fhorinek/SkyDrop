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
    this.data_holder = false;
    this.data_loading = false;
    this.data_loading_notify = [];
    this.data_load = 0;
   
    //Methods
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
            while (service.data_loading_notify.length)
            {
            	var deferred = service.data_loading_notify.pop();
                deferred.reject();
        	}            
        });
    };
    
    this.load_data = function(data)
    {
    	var deferred = $q.defer();
        
    	this.init_step_1(data, this);     
    	
    	return deferred.promise;
    };    

    this.load_json = function(url_path, cb)
    {
        var callback = cb;
        var service = this;
    
        console.log("loading bin resource %s", url_path);

        $http.get(url_path)     
        .success(function(data) {
            console.log("sucess");
            //console.log(data);
            callback(data, service);            
        })
        .error(function(xhr, status, error) {
            console.warn("error", xhr, status, error);
            if (error_cb != undefined)
            	error_cb();
            while (service.data_loading_notify.length)
            {
            	var deferred = service.data_loading_notify.pop();
                deferred.reject();
        	}   
      });
    };

    this.getType = function(key)
    {
        return this.ee_map[key][2];
    };
    
    this.getValue = function(key)
    {
        var mem_index;
        mem_index = this.ee_map[key][0];
        
        switch(this.getType(key))
        {
            case('uint8_t'):
                return get_uint8(this.ee_buffer, mem_index);
            
            case('int8_t'):
                return get_int8(this.ee_buffer, mem_index);

            case('uint16_t'):
                return get_uint16(this.ee_buffer, mem_index);
                
            case('int16_t'):
                return get_int16(this.ee_buffer, mem_index);

            case('uint32_t'):
                return get_uint32(this.ee_buffer, mem_index);

            case('float'):
                return get_float(this.ee_buffer, mem_index);
        }
    };
    
    //get from dataholder not ee_buffer
    this.getActualValue = function(key)
    {
    	for (var i in this.data_holder)
    		if (this.data_holder[i].pname == key)
    			return this.data_holder[i].value;
    	
    	return undefined;
    };
    
    this.setValue = function(key, value)
    {
        var mem_index = this.ee_map[key][0];
        var type = this.getType(key);
        
        console.log("set", value, "to", key, "type", type, "index", mem_index);
        
        switch(type)
        {
            case('uint8_t'):
                this.ee_buffer = set_uint8(this.ee_buffer, mem_index, value);
            break;
            
            case('int8_t'):
                this.ee_buffer = set_int8(this.ee_buffer, mem_index, value);
            break;
            
            case('uint16_t'):
                this.ee_buffer = set_uint16(this.ee_buffer, mem_index, value);
            break;
                
            case('int16_t'):
                this.ee_buffer = set_int16(this.ee_buffer, mem_index, value);
            break;
            
            case('uint32_t'):
                this.ee_buffer = set_uint32(this.ee_buffer, mem_index, value);
            break;

            case('float'):
                this.ee_buffer = set_float(this.ee_buffer, mem_index, value);
            break;                
        }
    };  

    //set to data holder not to ee buffer
    this.setActualValue = function(key, value)
    {
    	for (var i in this.data_holder)
    		if (this.data_holder[i].pname == key)
    			this.data_holder[i].value = value;
    	
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
            this.setValue(item.pname, item.value);
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
            
            //translate MACROS
            if (arr["mode"] == "flags")
            {
                for (var i=0; i < arr["flags"].length; i++)
                    if (isNaN(arr["flags"][i][0])) 
                        arr["flags"][i][0] = this.getMacroValue(arr["flags"][i][0]);
            }

            if (arr["mode"] == "select")
            {
                for (var i=0; i < arr["options"].length; i++)
                    if (isNaN(arr["options"][i][0])) 
                        arr["options"][i][0] = this.getMacroValue(arr["options"][i][0]);
            }

            if (arr["mode"] == "altimeter")
            {
                for (var i=0; i < arr["options"].length; i++)
                    if (isNaN(arr["options"][i][0])) 
                        arr["options"][i][0] = this.getMacroValue(arr["options"][i][0]);
                for (var i=0; i < arr["flags"].length; i++)
                    if (isNaN(arr["flags"][i][0])) 
                        arr["flags"][i][0] = this.getMacroValue(arr["flags"][i][0]);
            }            
            
            if (arr["mode"] == "doubleselect")
            {
                for (var i=0; i < arr["options1"].length; i++)
                    if (isNaN(arr["options1"][i][0])) 
                        arr["options1"][i][0] = this.getMacroValue(arr["options1"][i][0]);
                for (var i=0; i < arr["options2"].length; i++)
                    if (isNaN(arr["options2"][i][0])) 
                        arr["options2"][i][0] = this.getMacroValue(arr["options2"][i][0]);
            }              
            
            items[k] = (arr);
        }
        
        //notify watchers about the major change in structure -> need to rebind the data
        this.data_load++;
        
        return items;
    };


    this.getAllValues = function()
    {
        var deferred = $q.defer();
        
        if (this.data_holder == false)
        {
        	if (this.data_loading == false)
    		{
        		this.data_loading = true;
        		this.load_bin("UPDATE.EE", this.init_step_1);
    		}
        	this.data_loading_notify.push(deferred);
        }
        else
        {
            deferred.resolve(this.data_holder);
        }
        
        return deferred.promise;
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
        
        //load parameters description
        service.load_json("res/desc.json", service.init_step_3);
    };

    this.init_step_3 = function(data, service)    
    {
        service.ee_desc = data;
        
        service.data_holder = service.getAllValues_async();
        
        //console.log(">FRESH>", service.data_holder);
        while (service.data_loading_notify.length)
        {
        	var deferred = service.data_loading_notify.pop();
        	deferred.resolve(service.data_holder);
    	}
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
    
    this.getMacroValue = function(macro_name)
    {
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
    
    this.restore_default = function()
    {
    	var deferred = $q.defer();
    	
    	this.load_bin(this.fw_path + "UPDATE.EE", this.init_step_1, deferred);
    	
    	return deferred.promise;
    };
    
    this.is_old_version = function()
    {
    	return this.build_number != this.newest_build;    	
    };
    
    this.upgrade = function()
    {
    	console.log(this.data_holder);
    	
    	var old_values = {};
    	
    	//store old values
    	for (var key in this.data_holder)
    	{
    		var line = this.data_holder[key];
    		if (line.pname != "cfg_build_number")
    			old_values[line.pname] = line.value;
    	}
    	
    	console.log(old_values);
    	
    	//load newest fw
        var deferred = $q.defer();
        
        this.load_bin("UPDATE.EE", this.init_step_1, deferred);
        
        var service = this;
        
        deferred.promise.then(function (){
        	console.log(old_values);
        	//restore values
        	for (var key in old_values)
        	{
        		service.setActualValue(key, old_values[key]);
        	}
        });    	
    };
    
}]);

