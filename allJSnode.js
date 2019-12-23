function allmesh(e) {
    var n = "?" + e.name + "=" + e.checked,
        t = new XMLHttpRequest;
    t.open("GET", n, !1), t.send(n)
}