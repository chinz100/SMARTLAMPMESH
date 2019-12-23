function findnode(e) {
    var n = e.name + "=" + e.checked,
        o = new XMLHttpRequest;
    o.open("POST", "choosenode", !1), o.setRequestHeader("Content-Type", "application/x-www-form-urlencoded"), o.send(n)
}