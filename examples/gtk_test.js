var gtk = require("./gtk");

gtk.init(0);

var win = new gtk.Window({type: gtk.WindowType.toplevel, title:"Node.JS GTK Window"});
var button = new gtk.Button();

win.setBorderWidth(10);

button.setLabel("hallo, welt!");

win.add(button);
win.showAll();

var w2 = button.getParentWindow();
console.log(w2);

win.connect("destroy", function() {
    console.log("destroyed");
    gtk.mainQuit();
});

let clicked_count = 0;
button.connect("clicked", function() {
    console.log('button clicked');
    button.setLabel(`clicked: ${clicked_count++} times`);
});

console.log(win.setProperty("name", "test"));
console.log(win.__get_property__("name"));

gtk.main();
