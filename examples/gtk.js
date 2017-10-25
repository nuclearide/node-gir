const { Gtk } = require('../');

const win = new Gtk.Window({
    type: Gtk.WindowType.toplevel,
    title:"Node.JS Gtk Window"
});
win.setBorderWidth(10);

const button = new Gtk.Button();
button.setLabel("hallo, welt!");
win.add(button);

win.connect("destroy", function() {
    Gtk.mainQuit();
});

let clicked_count = 0;
button.connect("clicked", function() {
    button.setLabel(`clicked: ${clicked_count++} times`);
});

win.showAll();
Gtk.main();
