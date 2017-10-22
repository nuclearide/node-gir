var gtk = require("./gtk");

gtk.init(0);

var win = new gtk.Window({type: gtk.WindowType.toplevel, title:"Node.JS GTK Window"});
var button = new gtk.Button();

win.set_border_width(10);

button.set_label("hallo, welt!");

win.add(button);
win.show_all();

var w2 = button.get_parent_window();
console.log(w2);

win.on("destroy", function() {
    console.log("destroyed");
    gtk.main_quit();
});

let clicked_count = 0;
button.on("clicked", function() {
    button.set_label(`clicked: ${clicked_count++} times`);
});

console.log(win.set_property("name", "test"));
console.log(win.__get_property__("name"));

gtk.main();
