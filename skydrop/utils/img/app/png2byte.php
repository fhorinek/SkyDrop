<?

$invert = isset($_POST["invert"]) ? true : false;

?>

<html>
<body>

<form action="?" method="post" enctype="multipart/form-data">
<label for="file">Filename only PNG:</label>
<input type="file" name="file" id="file"><br>
<input type="checkbox" name="invert" id="invert" value="1" <?=(($invert) ? "checked" : "") ?>><label for="invert">Invert colors</label>
<input type="submit" name="submit" value="Submit">
</form>

<?

if (!isset($_FILES["file"]))
    die();

if ($_FILES["file"]["error"] > 0)
  {
  echo "Error: " . $_FILES["file"]["error"] . "<br>";
  die();
  }
else
  {
  echo "Upload: " . $_FILES["file"]["name"] . "<br>";
  echo "Type: " . $_FILES["file"]["type"] . "<br>";
  echo "Size: " . ($_FILES["file"]["size"] / 1024) . " kB<br>";
  echo "Stored in: " . $_FILES["file"]["tmp_name"];
  }

$name = $_FILES["file"]["tmp_name"];
$im = imagecreatefrompng($name);
$size = getimagesize($name);
$w = $size[0];
$h = $size[1];



$bytes = $w.", ".round(ceil($h/8.0) * 8).", ";

for ($r = 0; $r < ceil($h/8.0); $r++)
{
    for ($x=0; $x<$w; $x++)
    {
        $byte = 0;
        for ($i=0;$i<8;$i++)
        {
            $y = 8*$r + $i;
            if ($x >= $w || $y >= $h)
                $rgb = 1;
            else
                $rgb = imagecolorat($im, $x, $y);
                
                
            if ($invert)
            {
                if ($rgb == 0)
                    $byte |= (1 << $i);                
            }
            else
            {
                if ($rgb != 0)
                    $byte |= (1 << $i);                
            }
        }
        $bytes .= $byte.", "; 
    }
}

echo "<br><div style='border:1px solid black; width:80%; padding: 30px;'><tt>";
echo $bytes;
echo "</tt></div>";


?>

</body>
</html>
