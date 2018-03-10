const { Gtk } = require('../');

const builder = new Gtk.Builder();
builder.addFromFile("builder.ui");

const win = builder.getObject("window");
win.connect('destroy', () => {
    Gtk.main_quit();
})
const button1 = builder.getObject('button1');
button1.connect('clicked', () => {
    console.log("Hello World");
})

const button2 = builder.getObject('button2');
button2.connect('clicked', () => {
    console.log("Hello World");
})

const quit_button = builder.getObject('quit');
quit_button.connect('clicked', () => {
    Gtk.main_quit();
})

Gtk.main();
