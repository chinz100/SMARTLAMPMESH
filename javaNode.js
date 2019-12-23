setInterval(function() {
getData('info');
}, 5000);
function onoff(el) {
var data = (el.name + '=' + el.checked);
var request = new XMLHttpRequest();
request.open('POST', 'Sendstatnode', false);
request.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
request.send(data);
}
function getData(uri) {
var xhttp = new XMLHttpRequest();
xhttp.onreadystatechange = function() {
if (this.readyState == 4 && this.status == 200) {
var jj = JSON.parse(this.responseText);

for (var i = 0; i < jj.length; i++) {
j = jj[i];
if (document.getElementById(j.name)) {
j.name === 'led0'
? (document.getElementById(j.name).checked = j.val)
: (document.getElementById(j.name).innerHTML = j.val);
}

if (document.getElementById(j.name)) {
j.name === 'led1'
? (document.getElementById(j.name).checked = j.val)
: (document.getElementById(j.name).innerHTML = j.val);
}

}
}
};
xhttp.open('GET', uri, false);
xhttp.send();
}
