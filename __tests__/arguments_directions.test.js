const { load, Gtk } = require('../');

const GdkPixbuf = load('GdkPixbuf');

const window = new Gtk.Window({
  type: Gtk.WindowType.TOPLEVEL,
  title: 'Node.JS GTK Window'
});

describe('Arguments direction', () => {
  describe('in', () => {
    test('integer', () => {
      window.setProperty('default_height', 1);
    });

    test('string', () => {
      window.setProperty('title', 'Lancelot');
    });

    test('boolean', () => {
      window.setProperty('modal', true);
    });

    test('double', () => {
      window.setProperty('opacity', 0.5);
    });

    test('null', () => {
      window.setIcon(null);
    });

    test('object', () => {
      const pixbuf = new GdkPixbuf.Pixbuf();
      window.setIcon(pixbuf);
    });
  });

  describe('out', () => {
    // test('out arguments', () => {
    //   window.setTitle("Lancelot");
    //   expect(window.getProperty("title")).toEqual("Lancelot");
    // });
    // test('integer', () => {
    //   const type = GObject.typeFromName("GtkWindow");
    //   const children = GObject.typeChildren(type); // hidden, implicit array length
    //   expect(children.length).not.toBeLessThan(1);
    //   expect(children).toContain(GObject.typeFromName('GtkDialog'));
    //   expect(children).toContain(GObject.typeFromName('GtkAssistant'));
    //   expect(children).toContain(GObject.typeFromName('GtkPlug'));
    //   expect(children).toContain(GObject.typeFromName('GtkOffscreenWindow'));
    // });
  });
});
