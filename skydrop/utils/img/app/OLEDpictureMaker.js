
var tempColor = new Array();
var buffer = new Array();
var isMouseDown = false;
var selectedTool = "pencil";
var selectedSize = "size1";
var canvasWidth = 84;
var canvasHeight = 48;

function main(){
  ClearBufferArray();
  ClearTempColorArray();
  SelectPencil();
  SelectSize1();
  CreatePixelBoard();

  var load_popup = document.getElementById("load_array");
  load_popup.style.display = "none"; 

  load_popup = document.getElementById("OLEDpopup");
  load_popup.style.display = "none"; 

}

function HighlightCell(cell){
  if (isMouseDown == false){
    saveTempPixelData(cell);
  }
  
  if (selectedSize == "size1")
    cell.style.backgroundColor = "#008CFF";
  else if (selectedSize == "size2"){
    var pix = getCellCoords(cell);
                   
    cell.style.backgroundColor = "#008CFF";
    

    if ((eval(pix.x)+1) <= eval(canvasWidth-1)) {
      var id = "pixel*"+(eval(pix.x)+1)+"*"+pix.y;
      var _tmpcell = document.getElementById(id);
      _tmpcell.style.backgroundColor = "#008CFF";
    }
    if ((eval(pix.y)+1) <= eval(canvasHeight-1)) {
       var id = "pixel*"+pix.x+"*"+(eval(pix.y)+1);
       var _tmpcell = document.getElementById(id);
       _tmpcell.style.backgroundColor = "#008CFF";
    }
    if ((eval(pix.x)+1) <= eval(canvasWidth-1) && (eval(pix.y)+1) <= eval(canvasHeight-1)) {
       var id = "pixel*"+(eval(pix.x)+1)+"*"+(eval(pix.y)+1);
       var _tmpcell = document.getElementById(id);
       _tmpcell.style.backgroundColor ="#008CFF";
    }
  }
  
  UpdateStatusBar(cell);
}

function saveTempPixelData(cell){
    if (selectedSize == "size1")
      tempColor[0] = cell.style.backgroundColor;
    else if (selectedSize == "size2"){
      var pix = getCellCoords(cell);
      
      tempColor[0] = cell.style.backgroundColor;
      if ((eval(pix.x)+1) <= eval(canvasWidth-1)) {
        var id = "pixel*"+(eval(pix.x)+1)+"*"+pix.y;
        tempColor[1] = document.getElementById(id).style.backgroundColor;
      }
      else tempColor[1] = "";
      if ((eval(pix.y)+1) <= eval(canvasHeight-1)) {
        var id = "pixel*"+pix.x+"*"+(eval(pix.y)+1);
        tempColor[2] = document.getElementById(id).style.backgroundColor;
      }
      else tempColor[2] = "";
      if ((eval(pix.x)+1) <= eval(canvasWidth-1) && (eval(pix.y)+1) <= eval(canvasHeight-1)) {
        var id = "pixel*"+(eval(pix.x)+1)+"*"+(eval(pix.y)+1);
        tempColor[3] = document.getElementById(id).style.backgroundColor;
      }
      else tempColor[3] = "";
    }
}

function UpdateStatusBar(cell){
  var pix = getCellCoords(cell);    
  var status = "X:"+pix.x+" Y:"+pix.y;   
  document.getElementById('statusBar').innerHTML = status;
}

function UnHighlightCell(cell){
  if (selectedSize == "size1")
    cell.style.backgroundColor = tempColor[0];
  else if (selectedSize == "size2"){
    var pix = getCellCoords(cell);
                   
    cell.style.backgroundColor = tempColor[0];
    if ((eval(pix.x)+1) <= eval(canvasWidth-1)){
      var id = "pixel*"+(eval(pix.x)+1)+"*"+pix.y;
      var _tmpcell = document.getElementById(id);
      _tmpcell.style.backgroundColor = tempColor[1];
    }
    if ((eval(pix.y)+1) <= eval(canvasHeight-1)){
      var id = "pixel*"+pix.x+"*"+(eval(pix.y)+1);
      var _tmpcell = document.getElementById(id);
      _tmpcell.style.backgroundColor = tempColor[2];
    }
    if ((eval(pix.x)+1) <= eval(canvasWidth-1) && (eval(pix.y)+1) <= eval(canvasHeight-1)){
      var id = "pixel*"+(eval(pix.x)+1)+"*"+(eval(pix.y)+1);
      var _tmpcell = document.getElementById(id);
      _tmpcell.style.backgroundColor = tempColor[3];
    }
  }
}

function SetPixel(cell){
  var color;
  
  if(isMouseDown == false) return;
  
  if (selectedTool == "pencil")
    color = "black";
  else
    color = "white";
  
  tempColor[0] = color;
  tempColor[1] = color;
  tempColor[2] = color;
  tempColor[3] = color;
    
  var pix = getCellCoords(cell); 
  var result = get_pixel_color(cell);
  var result1 = (color == "" || color == "white")?0:1;

  if (result == result1 && selectedSize == "size1")
	return;
  if (result != result1)
  	PutPixelToBuffer(eval(pix.x),eval(pix.y),(color == "" || color == "white")?0:1);

  if (selectedSize == "size1") {
    cell.style.backgroundColor = color;
  }
  else if (selectedSize == "size2"){                   
    cell.style.backgroundColor = color;

    var id = document.getElementById("pixel*"+(eval(pix.x)+1)+"*"+pix.y);
    result = get_pixel_color(id );
    result1 = (color == "" || color == "white")?0:1;

    if ((eval(pix.x)+1) <= eval(canvasWidth-1) && result != result1){
      var id = "pixel*"+(eval(pix.x)+1)+"*"+pix.y;
      PutPixelToBuffer((eval(pix.x)+1),eval(pix.y),(color == "" || color == "white")?0:1);
      document.getElementById(id).style.backgroundColor = color;
    }

    var id = document.getElementById("pixel*"+eval(pix.x)+"*"+(eval(pix.y)+1));
    result = get_pixel_color(id );
    result1 = (color == "" || color == "white")?0:1;

    if ((eval(pix.y)+1) <= eval(canvasHeight-1) && result != result1){
      var id = "pixel*"+eval(pix.x)+"*"+(eval(pix.y)+1);
      PutPixelToBuffer(eval(pix.x),(eval(pix.y)+1),(color == "" || color == "white")?0:1);
      document.getElementById(id).style.backgroundColor = color;
    }

    var id = document.getElementById("pixel*"+(eval(pix.x)+1)+"*"+(eval(pix.y)+1));
    result = get_pixel_color(id );
    result1 = (color == "" || color == "white")?0:1;

    if ((eval(pix.x)+1) <= eval(canvasWidth-1) && result != result1){
      var id = "pixel*"+(eval(pix.x)+1)+"*"+(eval(pix.y)+1);
      PutPixelToBuffer((eval(pix.x)+1),(eval(pix.y)+1),(color == "" || color == "white")?0:1);
      document.getElementById(id).style.backgroundColor = color;
    }
  }

}

function get_pixel_color(cell){

	var pix = getCellCoords(cell);
	var x = pix.x;
	var y = pix.y; 
	var index = eval(Math.floor(eval(y / 8)) * canvasWidth) + eval(x % canvasWidth);
	var bin = buffer[index];
	bin = bin.toString(2);
	bin = bin.split("").reverse().join("");
	var position = y % 8;

	if(position >= bin.length)
		return 0;


	return bin[position];
	
}

function PutPixelToBuffer(x ,y ,color)
{  
	var index = eval(Math.floor(eval(y / 8)) * canvasWidth) + eval(x % canvasWidth);

	if (color == 1){
    		var num = (1 << eval(y % 8));
		buffer[index] += num ;
		buffer[index] = (eval(buffer[index]) | num);
  	}else{
		var bin = buffer[index];
		bin = bin.toString(2);
		bin = bin.split("").reverse().join("");
		var position = y % 8;	
		bin = replace_char(position,'0',bin);
		bin = bin.split("").reverse().join("");
		bin = parseInt(bin , 2);
    		buffer[index] = bin;
  	}

}

function replace_char(index, character,string) {
      return string.substr(0, index) + character + string.substr(index+character.length);
}

function SelectPencil(){
  var string = "<b>Tools</b><br/><img src=\"pencil_on.png\" onmousedown=\"\"/><img src=\"eraser_off.png\" onmousedown=\"SelectEraser();\"/>"; 
  document.getElementById('toolsUI').innerHTML = string;
  
  selectedTool = "pencil";
  isMouseDown = false;
}

function SelectEraser(){
   var string = "<b>Tools</b><br/><img src=\"pencil_off.png\" onmousedown=\"SelectPencil();\"/><img src=\"eraser_on.png\" onmousedown=\"\"/>"; 
  document.getElementById('toolsUI').innerHTML = string;
  
  selectedTool = "eraser";
  isMouseDown = false;
}

function SelectSize1(){
  var string = "<b>Size</b><br/><img src=\"size1_on.png\" onmousedown=\"\"/><img src=\"size2_off.png\" onmousedown=\"SelectSize2();\"/>"; 
  document.getElementById('sizeUI').innerHTML = string;
  
  selectedSize = "size1";
  isMouseDown = false;
}

function SelectSize2(){
   var string = "<b>Size</b><br/><img src=\"size1_off.png\" onmousedown=\"SelectSize1();\"/><img src=\"size2_on.png\" onmousedown=\"\"/>"; 
  document.getElementById('sizeUI').innerHTML = string;
  
  selectedSize = "size2";
  isMouseDown = false;
}

function NewDocument() {
  
  if (isDocumentEmpty() == false){
    var answer = confirm("Do you want to save changes ?")
  	if (answer){
  		SaveDocument();
  	}
  }
  getNewCanvasDimension();
  CreatePixelBoard();
  ClearBufferArray();
}

function SaveDocument(){
  var popup = document.getElementById("OLEDpopup");
  var innderPopup = document.getElementById("innerPopup");
  
  innerPopup.value = createCArray();
  popup .style.display = "block"; 
}

function load_up(){

    ClearBufferArray();
    ClearTempColorArray();
    SelectPencil();
    SelectSize1();
    CreatePixelBoard();
  
    var values_strings = document.getElementById("load_them_up_area").value;
    document.getElementById("load_them_up_area").value = '';
    	values_strings = values_strings.replace("{","");
    	values_strings = values_strings.replace("}","");
    	values_strings = values_strings.replace(" ","");
    
	var pixelies;
	
	isMouseDown=true;
	
	var string = values_strings.split(',');

	var count = 2;
	var x = 0;
	var y = 0;
	var array1 = new Array("128",'64','32','16','8','4','2','1');
	var value;
	var bin;
	var string;


	while (count < string.length)
	{

		value = parseInt(string[count]);
		
		if (value != 0)
		{
		
			bin = value.toString(2);
			bin = bin.split("").reverse().join("");
			
			for (var i=0; i < bin.length; i++) 
			{
				if (parseInt(bin.charAt(i)) == 1)
				{
					pixelies = document.getElementById("pixel*" + (x + '') + ('*'+(y+i)));
					SetPixel(pixelies);
				}
					
			}
		}			
			
		count += 1;
		x += 1;
		
		if(x >= canvasWidth){
			x = 0;
			y += 8;
		}
			
	}

	isMouseDown=false;
    var load_popup = document.getElementById("load_array");
    load_popup.style.display = "none"; 
    isMouseDown=false;
}


function LoadDocument(){
  var load_popup = document.getElementById("load_array");
  if(load_popup.style.display == 'none')
	 load_popup.style.display = "block"; 
  else
 	load_popup.style.display = "none"; 

}

function HidePopup(){
  var popup = document.getElementById("OLEDpopup");
  popup.style.display = "none";
}

function createCArray(){
var length = GetBufferArrayLength();
var collumns = Math.floor(canvasHeight/8);
 
if (collumns != eval(canvasHeight/8)) {
  collumns = collumns + 1;
}

var string = "uint8_t image["+eval(length+2)+"]={"+canvasWidth+","+eval(collumns*8)+",";

for(var i=0;i<length;i++){
  string = string + buffer[i];
  if (i < eval(length-1)) string = string + ",";
}

string = string + "};";

return string;
}

function isDocumentEmpty(){
  var isEmpty = true;
  
  for(var x=0; x < canvasWidth; x++){
    for(var y=0; y < canvasHeight; y++){
      var pixel = "pixel*"+x+"*"+y;
      var cell = document.getElementById(pixel);
      if (cell.style.backgroundColor != null) {
        isEmpty = false;                               
        break;
      }
    }
    
    if (isEmpty == true) break;
  }
  
  return isEmpty;
}

function getNewCanvasDimension(){  
  canvasWidth = document.getElementById('width').value;
  canvasHeight = document.getElementById('height').value;
  
  if (isNaN(canvasWidth) || ((canvasWidth < 0) || (canvasWidth > 84))){
    canvasWidth = 84;
    document.getElementById('width').value = "84";
  }
  if (isNaN(canvasHeight) || ((canvasHeight < 0) || (canvasHeight > 48))){
    canvasHeight = 48;
    document.getElementById('height').value = "48";
  }
}

function ClearTempColorArray(){
  tempColor[0] = "";
  tempColor[1] = "";
  tempColor[2] = "";
  tempColor[3] = "";
}

function ClearBufferArray(){
 var length = GetBufferArrayLength();
 
 for (var i=0;i<length;i++) buffer[i] = 0;
}

function GetBufferArrayLength(){
  var length;
 var  rows, collumns;
 
 rows = canvasWidth;
 collumns = Math.floor(canvasHeight/8);
 
 if (collumns != eval(canvasHeight/8)) {
    collumns = collumns + 1;
 }
 
 length = rows * collumns;
 return length;
}

function ClearPixelBoard(){
  for(var x=0; x < canvasWidth; x++){
    for(var y=0; y < canvasHeight; y++){
      var pixel = "pixel*"+x+"*"+y;
      document.getElementById(pixel).style.backgroundColor = "white";
      }
    }
}

function CreatePixelBoard(){   
     var x = 0,y = 0;
     var result = '';       
     
     var canvasWrapper = document.getElementById('canvasWrapper');
     
     var wrapperHeight = document.getElementById('wrapper').clientHeight;
     var UIheight = document.getElementById('userInterface').clientHeight;
     var statusHeight = document.getElementById('statusBar').clientHeight;
     canvasWrapper.style.width = eval(canvasWidth*4)+"px";
     
     var topPadding = Math.floor(eval(wrapperHeight-UIheight-statusHeight-eval(canvasHeight*4))/2);
     if (topPadding < 1) topPadding = 0;
     canvasWrapper.style.marginTop = topPadding+"px";
     canvasWrapper.style.paddingBottom = topPadding+"px";
                        
     for(y=0;y<canvasHeight;y++){
        result = result + "<div unselectable=\"on\" style=\"display:block;margin-top:0px\">";
        for(x=0;x<canvasWidth;x++){
          result = result + "<div id=\"pixel*"+x+"*"+y+"\" unselectable=\"on\" class=\"pixel\" onmouseup=\"isMouseDown=false;\" onmousedown=\"isMouseDown=true;SetPixel(this);\" onmouseover=\"HighlightCell(this);SetPixel(this);\" onmouseout=\"UnHighlightCell(this);\"></div>";
        }
        result = result + "</div>";
     }

     document.getElementById('canvasWrapper').innerHTML = result;
}

function getCellCoords(cell){
  var string = cell.id;
  var n = string.split("*");
  
  var pix = new Object();
  pix.x = n[1];
  pix.y = n[2];
  
  return pix;
}