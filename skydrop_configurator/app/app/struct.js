'use strict';

function zero_pad(padding, number)
{
    var str;
    
    str = number.toString();
    while (str.length < padding)
        str = "0" + str;
        
    return str;
}

function in_range(value, vmin, vmax)
{
    if (value > vmax || value < vmin)
    {
        console.trace();
        console.warn("Value out of bounds!", value, vmin, vmax);
        
        return vmin;
    }
    
    return Math.round(value);
}

function get_char(data, index)
{
	return String.fromCharCode(data[index])
}

function set_char(data, index, value, size)
{
	for (var i = 0; i < value.length; i++)
	{
		data[index + i] = value.charCodeAt(i);
		if (i >= size - 2)
			break;
	}
		
	//end with \0
	if (value.length < size - 1)
		data[index + value.length] = 0;
	else
		data[index + size - 1] = 0;
    
    return data;
}



function get_uint8(data, index)
{
    return data[index];
}


function set_uint8(data, index, value)
{
    value = in_range(Number(value), 0, 255);

    data[index] = value;
    
    return data;
}


function get_int8(data, index)
{
    var buffer = new ArrayBuffer(1);
    var byte = new Int8Array(buffer);
    
    byte[0] = data[index + 0];
    
    var int8 = new Int8Array(buffer);
    
    return int8[0];
}

function set_int8(data, index, value)
{
    var buffer = new ArrayBuffer(1);
    var byte = new Int8Array(buffer);
    var int8 = new Int8Array(buffer);
    
    int8[0] = value;
    
    data[index + 0] = byte[0];
    
    return data;
}

function get_uint16(data, index)
{

    var buffer = new ArrayBuffer(2);
    var byte = new Uint8Array(buffer);

    byte[0] = data[index + 0];
    byte[1] = data[index + 1];    

    var val = new Uint16Array(buffer);  
    
    return val[0];
}

function set_uint16(data, index, value)
{
    var buffer = new ArrayBuffer(2);
    var uint16 = new Uint16Array(buffer);
    var byte = new Uint8Array(buffer);
    
    uint16[0] = value;
    
    data[index + 0] = byte[0];
    data[index + 1] = byte[1];
    
    return data;
}

function get_uint32(data, index)
{

    var buffer = new ArrayBuffer(4);
    var byte = new Uint8Array(buffer);

    byte[0] = data[index + 0];
    byte[1] = data[index + 1];    
    byte[2] = data[index + 2];    
    byte[3] = data[index + 3];    

    var val = new Uint32Array(buffer);  
    
    return val[0];
}

function set_uint32(data, index, value)
{
    var buffer = new ArrayBuffer(4);
    var uint32 = new Uint32Array(buffer);
    var byte = new Uint8Array(buffer);
    
    uint32[0] = value;
    
    data[index + 0] = byte[0];
    data[index + 1] = byte[1];
    data[index + 2] = byte[2];
    data[index + 3] = byte[3];
    
    return data;
}

function get_float(data, index)
{
    var buffer = new ArrayBuffer(4);
    var byte = new Uint8Array(buffer);
    
    byte[0] = data[index + 0];
    byte[1] = data[index + 1];
    byte[2] = data[index + 2];
    byte[3] = data[index + 3];
    
    var float = new Float32Array(buffer);
    
    return float[0];
}

function set_float(data, index, value)
{
    var buffer = new ArrayBuffer(4);
    var float = new Float32Array(buffer);
    var byte = new Uint8Array(buffer);
    
    float[0] = value;
    
    data[index + 0] = byte[0];
    data[index + 1] = byte[1];
    data[index + 2] = byte[2];
    data[index + 3] = byte[3];
    
    return data;
}

function get_int16(data, index)
{
    var buffer = new ArrayBuffer(2);
    var byte = new Uint8Array(buffer);
    
    byte[0] = data[index + 0];
    byte[1] = data[index + 1];
    
    var int = new Int16Array(buffer);
    
    return int[0];
}

function set_int16(data, index, value)
{
    var buffer = new ArrayBuffer(2);
    var int16 = new Int16Array(buffer);
    var byte = new Uint8Array(buffer);
    
    int16[0] = value;
    
    data[index + 0] = byte[0];
    data[index + 1] = byte[1];
    
    return data;
}
