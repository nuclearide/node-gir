const { load } = require('../');

const gtk = load('Gtk', '3.0');
const WebKit = load('WebKit', '3.0');

gtk.init(0);

const win = new gtk.Window();

win.on('destroy', () => {
  gtk.mainQuit();
  process.exit();
});

const sw = new gtk.ScrolledWindow();
win.add(sw);

const view = new WebKit.WebView();
view.loadUri('http://www.google.com/');
sw.add(view);

win.setSizeRequest(640, 480);
win.showAll();

gtk.main();
