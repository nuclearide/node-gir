const { load, Gtk } = require('../');

const GdkPixbuf = load('GdkPixbuf');

const pixbuf = new GdkPixbuf.Pixbuf();
const win = new Gtk.Window({
  type: Gtk.WindowType.TOPLEVEL,
  title: 'Node.JS GTK Window'
});

describe('Gtk.Object', () => {
  describe('property', () => {
    describe('boolean', () => {
      it('should be true', () => {
        win.modal = true;
        expect(win.modal).toBe(true);
      });

      it('should be false', () => {
        win.modal = false;
        expect(win.modal).toBe(false);
      });
    });

    describe('string', () => {
      it('set Lancelot', () => {
        win.title = 'Lancelot';
        expect(win.title).toEqual('Lancelot');
      });

      it('get Lancelot', () => {
        expect(win.title).toEqual('Lancelot');
        expect(win.title).not.toEqual('');
        expect(win.title).not.toEqual(' ');
      });
    });

    describe('integer', () => {
      it('set 1', () => {
        win['default-height'] = 1;
        expect(win['default-height']).toEqual(1);
      });

      it('get 1', () => {
        expect(win['default-height']).toEqual(1);
        expect(win['default-height']).not.toEqual(0);
        expect(win['default-height']).not.toEqual(-1);
      });
    });

    // TODO: see issue https://github.com/Place1/node-gir/issues/2
    // describe('double', function() {
    //     it('set/get 0.33', function() {
    //         win.opacity = 0.33;
    //         expect(win.opacity).toEqual(0.33);
    //     });
    // });

    describe('object', () => {
      it('set icon', () => {
        win.icon = pixbuf;
      });

      it('get icon', () => {
        expect(typeof (win.icon)).toEqual('object');

        // FIXME: the bindings crash if we use .toEqual because
        // toEqual recursively checks object properties and our bindings
        // crash in GIRObject::PropertyGetHandler!
        expect(win.icon).not.toBe(win);
      });
    });
  });
});
