function createXHR() 
{
    var xhr;
    try 
    {
        xhr = new ActiveXObject("Msxml2.XMLHTTP");
    } 
    catch (e) 
    {
        try 
        {
            xhr = new ActiveXObject("Microsoft.XMLHTTP");
        }
        catch(E) 
        {
            xhr = false;
        }
    }
 
    if (!xhr && typeof XMLHttpRequest != 'undefined') 
    {
        xhr = new XMLHttpRequest();
    }
    return xhr;
}
