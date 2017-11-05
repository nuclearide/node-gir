const { load, Gtk } = require('../');

const GObject = load('GObject');
const GdkPixbuf = load('GdkPixbuf');

const pixbuf = new GdkPixbuf.Pixbuf(0, false, 1, 1, 1);

describe('functions', () => {
  describe('functions can be called', () => {
    test('static methods should be callable', () => {
      const button = Gtk.Button.newWithLabel('my button'); // static method
      expect(button).not.toBe(undefined);
      expect(button.label).toEqual('my button');
    });

    test('object methods should be callable', () => {
      const button = new Gtk.Button();
      button.setLabel('my label'); // object method
      expect(button.label).toEqual('my label');
    });
  });

  describe('functions can return values', () => {
    test('functions can return a: number', function () {
      const intValue = GObject.typeFromName('GtkWindow');
      expect(typeof(intValue)).toEqual('number');
    });

    test('functions can return a: string', function () {
      const window = new Gtk.Window();
      window.title = 'Lancelot';
      expect(window.title).toEqual('Lancelot');
      expect(typeof (window.title)).toEqual('string');
    });

    test('functions can return a: boolean', function () {
      const window = new Gtk.Window();
      const decorated = window.getDecorated();
      expect(typeof (decorated)).toEqual('boolean');
    });

    test('functions can return: null', function () {
      const window = new Gtk.Window();
      window.setIcon(null);
      expect(window.getIcon()).toBeNull();
    });

    test('functions can return an: object', function () {
      const window = new Gtk.Window();
      window.icon = pixbuf;
      expect(typeof (window.icon)).toEqual('object');
    });

    test('functions can return: void', function () {
      const window = new Gtk.Window();
      const voidValue = window.resize(10, 10);
      expect(voidValue).toBeUndefined();
    });

    test('functions can return an: array', function () {
      const gtype = GObject.typeFromName('GtkWindow');
      const gtypeArray = GObject.typeChildren(gtype);
      expect(gtypeArray.length).not.toEqual(0);
    });
  });
});
