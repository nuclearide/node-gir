const { load, Gtk } = require('../');

var GdkPixbuf = load('GdkPixbuf');
var GObject = load('GObject');

var win = new Gtk.Window({
  type: Gtk.WindowType.TOPLEVEL,
  title: "Node.JS GTK Window"
});

describe('Arguments direction', function() {

  describe('in', function() {

    // it('integer', function() {
    //   win.setProperty("default_height", 1);
    // });

    // it('string', function() {
    //   win.setProperty("title", "Lancelot");
    // });

    // it('boolean', function() {
    //   win.setProperty("modal", true);
    // });

    it('double', function() {
      win.setProperty("opacity", 0.5)
    });

    // it('null', function() {
    //   win.setIcon(null);
    // });

    // it('object', function() {
    //   var pixbuf = new GdkPixbuf.Pixbuf();
    //   win.setIcon(pixbuf);
    // });
  });

  // describe('out', function() {

  //   /* FIXME: Fails with "Error: IN arguments conversion failed" */
  //   function pendingGetPropertyTest() {
  //     win.setTitle("Lancelot");
  //     var title = null;
  //     win.getProperty("title", title);
  //     expect(title).toEqual("Lancelot");
  //   };

  //   it('integer', function() {
  //     const type = GObject.typeFromName("GtkWindow");
  //     const children = GObject.typeChildren(type); // hidden, implicit array length
  //     expect(children.length).not.toBeLessThan(1);
  //     expect(children).toContain(GObject.typeFromName('GtkDialog'));
  //     expect(children).toContain(GObject.typeFromName('GtkAssistant'));
  //     expect(children).toContain(GObject.typeFromName('GtkPlug'));
  //     expect(children).toContain(GObject.typeFromName('GtkOffscreenWindow'));
  //   });
  // });

});

