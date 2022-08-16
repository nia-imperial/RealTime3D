# adapted from lensfun project

"""This program reads Adobe LCP files and converts their content to one Lensfun
XML file.  It was tested against the LCP files shipped with “Adobe DNG
Converter 9.0” although it should work with other sets of LCP files, too.

This program assumes that one LCP file contains the data of exactly one lens.

It writes a single XML output file, by default into the personal DB directory.
While this directory has the highest priority, note that other files in this
directory – albeit highly unlikely – might contain entries that override data
in that XML file.
"""

import argparse
import copy
import glob
import os
import re
from xml.etree import ElementTree

parser = argparse.ArgumentParser(description="Convert LCP files to a Lensfun XML file.")
parser.add_argument("input_directory", default=".", nargs="?", metavar="path",
                    help="""path to the LCP files (default: ".")""")
parser.add_argument("--output", default=os.path.expanduser("~/.local/share/lensfun/_lcps.xml"),
                    help="Path of output file (default: ~/.local/share/lensfun/_lcps.xml).  "
                         "This file is overwritten silently.")
parser.add_argument("--db-path", help="Path to the lensfun database.  If not given, look in the same places as Lensfun.")
parser.add_argument("--prefer-lcp", action="store_true", help="Prefer LCP data over Lensfun data.")
args = parser.parse_args()


def indent(elem, level=0):
    """Indent the output ElementTree in-place by added whitespace so that it looks
    nicer in the flattened output.  Taken from the ElementTree webseite.  It
    was modified so that it produces even nicer output for Lensfun files.

    :param elem: the root element of an ElementTree tree
    :param level: the indentation level of the root element, in numbers of space
        characters.

    :type elem: xml.etree.ElementTree.Element
    :type level: int

    :return:
      The same tree but with added whitespace in its ``text`` and ``tail``
      attributes.

    :rtype: xml.etree.ElementTree.Element
    """
    i = "\n" + level * "    "
    if len(elem):
        if not elem.text or not elem.text.strip():
            elem.text = i + "    "
        for elem in elem:
            indent(elem, level + 1)
            if level == 0:
                elem.tail = "\n" + i + "    "
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
    else:
        if level and (not elem.tail or not elem.tail.strip()):
            elem.tail = i


def child_without_attributes(parent, name):
    """Return the first child element that doesn't have attributes.  This is used
    e.g. for the ``<model>`` tag in order to get the version without the
    ``lang`` attribute.

    :param parent: parent element of the element to be retrieved
    :param name: tag name of the element to be retrieved

    :type parent: ElementTree.Element
    :type name: str

    :return:
      The first ``<name>`` within ``<parent>`` without attributes.  ``None`` if
      none was found.

    :rtype: ElementTree.Element or NoneType
    """
    for child in parent.findall(name):
        if not list(child.attrib.keys()):
            return child


# Part I
#
# Reading the Lensfun database


class LensfunCamera:
    """One camera entry in Lensfun's database.  It is used to derive cropfactors of
    LCP entries from them, and the mounts of compact cameras.
    """

    def __init__(self, maker, model):
        self.maker, self.model = maker, model
        self.mount = None
        self.cropfactor = None


class LensfunLens:
    """One ``<lens>`` entry in Lensfun's database.  They are used to match LCP
    entries against them, and use them as a starting point for output entries
    in case of a match.

    :ivar element: the original Lensfun DB XML element
    :ivar normalized_model: the tokenized form of the lens model name; see
      `normalize_lens_model_name`
    :ivar cropfactor: the cropfactor of the calibration
    :ivar fixed_lens_mount: if not ``None``, the name of the compact camera
      mount of this lens
    :ivar chdk: whether this calibration refers to CHDK's DNGs.

    :type element: ElementTree.Element
    :type normalized_model: tuple of str
    :type cropfactor: float
    :type fixed_lens_mount: NoneType or str
    :type chdk: bool
    """

    model_name_token_regex = re.compile(r"[0-9][0-9.]*|\s+|[^\w\s]+|[^\W\d_]+")
    model_name_number_regex = re.compile(r"\s+((AB|[ABCGS])\d{2,3}([ENPS]|EM)?|272E[ENPS]|F004[ENS]?|o77)(?=\s|$)")
    model_name_f_regex = re.compile(r"(?<=\s)F(?=\d)")
    model_name_fixes = {
        "TAMRON 18-270mm F/3.5-6.3 DiII PZD B008S": "Tamron AF 18-270mm f/3.5-6.3 Di II VC PZD",
        "SIGMA 18-35mm F1.8 DC HSM A013": "Sigma 18-35mm f/1.8 DC HSM [A]",
        "SIGMA 30mm F1.4 DC HSM A013": "Sigma 30mm f/1.4 EX DC HSM",
        "Sony DT 55-200mm F4-5.6 SAM": "Sony AF DT 55-200mm f/4-5.6 SAM",
        "Canon EF 135mm f/2 L USM": "Canon EF 135mm f/2.0L USM",
        "Nikon AF NIKKOR 35mm f/2D": "Nikon AF Nikkor 35mm f/2.0D",
        "Nikon AF-S DX DX NIKKOR 18-300mm f/3.5-6.3G ED VR": "Nikon AF-S DX Nikkor 18-300mm f/3.5-6.3G ED VR",
        "Nikon NIKKOR 50mm f/1.2 AIS": "Nikon AI-S Nikkor 50mm f/1.2",
        "SIGMA 12-24mm F4.5-5.6 EX DG ASPHERICAL HSM": "Sigma 12-24mm f/4.5-5.6 EX DG USM",
        "SIGMA 17-70mm F2.8-4.5 DC MACRO HSM": "Sigma 17-70mm f/2.8-4.5 DC Macro",
        "SIGMA 24-105mm F4 DG OS HSM A013": "Sigma 24-105mm f/4.0 DG OS HSM [A]",
        "SIGMA 8mm F3.5 EX DG CIRCULAR FISHEYE": "Sigma 8mm f/3.5 EX DG Circular",
        "TAMRON SP AF 28-75mm F2.8 XR Di": "Tamron SP AF 28-75mm f/2.8 XR Di LD Aspherical (IF) Macro",
        "TAMRON XR DiII 18-200mm F3.5-6.3": "Tamron AF 18-200mm f/3.5-6.3 XR Di II LD Aspherical (IF) Macro",
        "Tokina AT-X 124 PRO DX 12-24mm F4(IF)": "Tokina 12-24mm f/4 AT-X 124 AF Pro DX",
        "smc PENTAX-DA 10-17mm F3.5-4.5 ED [IF] Fisheye zoom": "smc Pentax-DA Fish-Eye 10-17mm f/3.5-4.5 ED IF"}
    uppercase_token_regex = re.compile(r"\b(SIGMA|TAMRON|ELMAR|SUMMILUX|SUMMICRON|SUMMARIT|ELMARIT|VARIO|SUPER|"
                                       r"TRI|NOCTILUX|TELYT|ASPH|PRO|MACRO|"
                                       r"ASPHERICAL|CIRCULAR|DIAGONAL|FISHEYE|PENTAX)\b")

    def __init__(self, element):
        """
        :param element: the Lensfun DB <lens> element

        :type element: ElementTree.Element
        """
        self.element = element
        model = child_without_attributes(element, "model").text
        self.normalized_model = self.normalize_lens_model_name(model)
        self.cropfactor = float(element.find("cropfactor").text)
        first_mount = element.find("mount").text
        if first_mount[0].islower():
            self.fixed_lens_mount = first_mount
        else:
            self.fixed_lens_mount = None
        self.chdk = "chdk" in model.lower()

    @staticmethod
    def normalize_lens_model_name(name):
        """Tokenise the given name.  This routine helps to implement a lens
        model matching similar to Lensfun itself.

        :param name: model name to be tokenised.

        :type name: str

        :return:
          The tokens found in the name.  Singular punctuation and f's are
          discarded, as done by Lensfun.

        :rtype: tuple of str
        """
        return tuple(token for token in LensfunLens.model_name_token_regex.findall(name.lower())
                     if token != "f" and not token.isspace() and
                     (len(token) > 1 or token.isalnum() or token in "*+"))

    @classmethod
    def sanitize_lcp_lens_model_name(cls, name):
        """Corrects errors in lens model names typically found in LCP files.  This
        improves matching with Lensfun's lens model names.

        :param name: the original lens model name in the LCP file

        :type name: str

        :return:
          the corrected lens model name

        :rtype: str
        """
        try:
            return cls.model_name_fixes[name]
        except KeyError:
            result = []
            previous_match = None
            for match in cls.uppercase_token_regex.finditer(name):
                result.append(name[previous_match and previous_match.end() or 0:match.start()])
                result.append(match.group(0).capitalize())
                previous_match = match
            result.append(name[previous_match and previous_match.end() or 0:])
            name = "".join(result)
            name = name.replace("DiII", "Di II")
            name = name.replace("Leica ", "")
            name = name.replace("Voigtlander", "Voigtländer")
            name = cls.model_name_number_regex.sub("", name)
            name = cls.model_name_f_regex.sub("f/", name)
            return name

    def matches(self, names, cropfactor, fixed_lens_mount):
        """Returns a score indicating how well the given parameters match this
        lens.

        :param names: model names to be checked; since the LCP files contain
          more than one, you pass a list here
        :param cropfactor: the cropfactor derived from the LCP file
        :param fixed_lens_mount: the name of the fixed lens mount derived from
          the LCP file, using Lensfun's camera list

        :type names: list of str
        :type cropfactor: float
        :type fixed_lens_mount: str or NoneType

        :return:
          A score that measures the similarity of the lens data sets.  It is a
          tuple of numeric values in order to assure one best match (or none).
          A return value of (-1000, 0) indicates no match.

        :rtype: (float, float)
        """
        if self.fixed_lens_mount and fixed_lens_mount == self.fixed_lens_mount and not self.chdk:
            return (1000, 0)
        scores = []
        for name in names:
            normalized_model = list(self.normalized_model)
            try:
                for token in self.normalize_lens_model_name(name):
                    normalized_model.remove(token)
            except ValueError:
                continue
            unmatched_tokens = len(normalized_model)
            if unmatched_tokens < 4:
                scores.append(10 - unmatched_tokens)
                break
        else:
            return (-1000, 0)
        cropfactor_ratio = cropfactor / self.cropfactor
        if not 0.96 < cropfactor_ratio < 1.041:
            return (-1000, 0)
        else:
            scores.append(10 - 100 * abs(1 - cropfactor_ratio))
        return tuple(scores)


def read_lensfun_database():
    """Reads the Lensfun database and returns its content.  It obeys to
    Lensfun's rules of overriding entries.

    :return:
      All found cameras and lenses.  As for cameras, the result is a dict with
      the key `(maker, model)`, both in all-lowercase.  This makes lookup
      easier later.

    :rtype: dict mapping (str, str) to `LensfunCamera`, set of `LensfunLens`
    """
    lensfun_cameras = {}
    lensfun_lenses_dict = {}
    def crawl_directory(dirpath):
        for filepath in glob.glob(os.path.join(dirpath, "*.xml")):
            if not os.path.basename(filepath).startswith("_"):
                tree = ElementTree.parse(filepath).getroot()
                for element in tree.findall("camera"):
                    maker, model = child_without_attributes(element, "maker").text, \
                                   child_without_attributes(element, "model").text
                    lensfun_cameras[maker.lower(), model.lower()] = camera = LensfunCamera(maker, model)
                    camera.cropfactor = float(element.find("cropfactor").text)
                    camera.mount = element.find("mount").text
                for element in tree.findall("lens"):
                    lens = LensfunLens(element)
                    lensfun_lenses_dict[lens.normalized_model, lens.cropfactor] = lens

    paths_search_list = [args.db_path] if args.db_path else \
        ["/usr/share/lensfun", "/usr/local/share/lensfun", "/var/lib/lensfun-updates",
         os.path.expanduser("~/.local/share/lensfun/updates"),
         os.path.expanduser("~/.local/share/lensfun")]
    for path in paths_search_list:
        crawl_directory(path)
    return lensfun_cameras, set(lensfun_lenses_dict.values())

lensfun_cameras, lensfun_lenses = read_lensfun_database()
if not lensfun_cameras and not lensfun_lenses:
    print("Warning: No Lensfun database found.")


# Part II
#
# Reading the LCP files and merging the data with the Lensfun data


class FieldNotFoundError(Exception):
    """Raised by `LCPLens.read_field` if an element/atribute with the given
    name was not found, and no default was defined.
    """
    pass


class NoLCPDataUsed(Exception):
    """Raised anywhere in the methods of `LCPLens` (but only during the
    ``__init__`` call) in order to signal that this LCP entry must not be used
    for the output, e.g. because Lensfun already contains a full set of data
    for that lens.
    """
    pass


class NoFieldDefault:
    """Singleton used as a parameter default in `LCPLens.read_field` to distinguish
    between “no default provided” and any kind of default including ``None``.
    """
    pass


# Important LCP namespaces
camera_ns = "{http://ns.adobe.com/photoshop/1.0/camera-profile}"
rdf_ns = "{http://www.w3.org/1999/02/22-rdf-syntax-ns#}"
# This regex identifies garbage in the <Lens> tag in the LCP file.
unusable_lens_name_regex = re.compile(r"[-0-9.]+\s*mm(\s*f/?[-0-9.]+)?|Sigma Lens$", re.IGNORECASE)
# This regex is used for finding a focal length in a string.
focal_length_regex = re.compile(r"([0-9.]+)\s*mm\b", re.IGNORECASE)


class LCPLens:
    """An entry in the LCP database.  It is identical with a single LCP file.

    :ivar compact_cameras: This clas variable collects all entries for compact
      cameras that should go into the output because Lensfun doesn't define
      them.  Note that even with the ``--prefer-lcp`` option, Lensfun camera
      definitions are not overridden by the XML output (but only calibration
      data).

    :ivar old_format: Adobe has changed the schema of the LCP files at least
      twice over the years.  This makes three variants.  In particular, tags
      became attributes, and then, the <Description> element was dropped at
      some point.  This flag distinguishes between the first two variants,
      i.e. tags and attributes.

    :ivar calibration_entries: all calibration entries (elements) in the LCP
      file.  One entry refers to one set of EXIF data (focal length, aperture,
      distance) and can contain distortion, TCA, and vignetting data.

    :ivar xml_element: The XML element ready to be used for the output.  It is
      already generated in the constructor in order to be able to mark the
      instance as unfit for output by raising `NoLCPDataUsed`.

    :ivar lensfun_lens: The Lensfun lens object that represents the same lens
      and calibration cropfactor as this lens.

    :ivar camera_model: Model of the camera used for the calibration.  This is
      used for helping with determining the calibration cropfactor, and it is
      used for detecting data of compact cameras.

    :ivar maker: Lens maker name.

    :ivar model: Lens model name.  This is supposed to be the equivalent of
      Lensfun's ``<model>`` tag without language attribute.

    :ivar model_en: Verbose lens model name.  This is supposed to be the
      equivalent of Lensfun's ``<model lang="en">`` tag.

    :ivar raw: Whether this calibration data was taken from RAW images.  If
      ``False``, JPEG is assumed.

    :ivar cropfactor: The cropfactor used for the calibration.  This script
      tries hard in determining the cropfactor (unfortunately, it is not always
      included into the LCP files).  It all else fails, it is set to 1.

    :ivar fixed_lens_mount: The mount name if we have detected a fixed-lens
      camera.  If not ``None``, it is either an original Lensfun mount name, or
      a made-up unique compact camera mount name.

    :type compact_cameras: list of ElementTree.Element
    :type old_format: bool
    :type calibration_entries: list of ElementTree.Element
    :type xml_element: ElementTree.Element
    :type lensfun_lens: `LensfunLens` or ``NoneType``
    :type camera_model: str
    :type maker: str
    :type model: str
    :type model_en: str
    :type raw: bool
    :type cropfactor: float
    :type fixed_lens_mount: str or ``NoneType``
    """

    compact_cameras = []

    def __init__(self, filepath):
        """
        :param filepath: path to the LCP file

        :type filepath: str
        """
        tree = ElementTree.parse(filepath)
        self.old_format = bool(
            tree.findall(".//{http://ns.adobe.com/photoshop/1.0/camera-profile}Make"))
        self.calibration_entries = [self.get_description_element_maybe(entry) for entry in tree.getroot()[0][0][0][0]]
        self.read_first_entry()
        self.fix_focal_lengths()
        self.xml_element = self.xml_element()

    def get_description_element_maybe(self, element):
        """Get the proper element to read calibration data from.  This routine helps to
        support both the second and the third variant of the LCP file format.
        The second variant uses ``<Description>`` tags below
        ``<PerspectiveModel>``, while the third variant attaches the attributes
        directly to ``<PerspectiveModel>``.  For the “old format” (first
        variant), this routine does nothing.

        :param element: the calibration entry, typically an <li> element

        :type element: ElementTree.Element

        :return:
          the real element carrying the calibration data

        :rtype: ElementTree.Element
        """
        if self.old_format:
            return element
        description = element.find(rdf_ns + "Description")
        if description is None:
            return element
        else:
            return description

    def read_field(self, element, field_name, default=NoFieldDefault):
        """:param element: element into which the field is placed

        :param field_name: The name of the field to be read.  For LCP
          variant 1, this is the tag of a child element.  For LCP variants 1
          and 2, this is an attribute name.

        :param default: Default value if the field is not found.  Note that it
          is distinguished between “no default given” and “``None`` is the
          default”.

        :type element: ElementTree.Element
        :type field_name: str
        :type default: object

        :return:
          the content of the field, or if the field doesn't exist, the default
          if given

        :raises FieldNotFoundError: if the field does not exist and not default
          is given
        """
        try:
            if self.old_format:
                return element.find(camera_ns + field_name).text
            else:
                return element.attrib[camera_ns + field_name]
        except (KeyError, AttributeError):
            if default is not NoFieldDefault:
                return default
            raise FieldNotFoundError(field_name)

    @staticmethod
    def clean_lens_maker(model):
        """Returns the maker of the given lens.  Unfortunately, LCP files don't contain
        the maker of the lens explicitly, but it is needed for Lensfun XML
        output.  Besides, the exact maker is used in `guess_ilc_mounts` to
        guess for which mounts the lens is probably available.

        :param model: lens model name that contains the maker somewhere

        :type model: str

        :return:
          the make of this lens

        :rtype: str
        """
        model_lower = model.lower()
        if "pentax" in model_lower:
            return "Pentax"
        if model_lower.startswith("hero"):
            return "GoPro"
        if model_lower.startswith("iphone"):
            return "Apple"
        if model_lower.startswith("inspire"):
            return "DJI"
        if model_lower.startswith("schneider"):
            return "Schneider-Kreuznach"
        if model_lower.startswith("phase one"):
            return "Phase One"
        if model_lower.startswith("venus optics"):
            return "Venus Optics"
        if "mitakon" in model_lower:
            return "Mitakon"
        if "cgo2g" in model_lower:
            return "Yuneec"
        if model_lower.startswith("slr magic"):
            return "SLR Magic"
        if model_lower.startswith("dp") and "quattro" in model_lower:
            return "Sigma"
        if "voigtlander" in model_lower:
            return "Voigtländer"
        if "handevision" in model_lower:
            return "HandeVision"
        maker = model.split()[0].capitalize()
        return maker

    def make_model_en_prettier(self):
        """Makes the human-friendly lens model name prettier.  This happens after
        having matched agains Lensfun names, so in contrast to
        `LensfunLens.sanitize_lcp_lens_model_name`, this function only acts on
        ``model_en``, and only for nice GUI entries.
        """
        if "nikkor" in self.model_en.lower() and self.model_en.lower().startswith("nikon"):
            tokens = self.model_en.split()
            for i, token in enumerate(tokens):
                if "nikkor" in token.lower():
                    fisheye = "fisheye" in token.lower()
                    del tokens[i]
                    if fisheye:
                        if tokens[-1] == "(JPEGs)":
                            tokens.insert(-1, "Fisheye")
                        else:
                            tokens.append("Fisheye")
                    break
            try:
                dx_index = tokens.index("DX")
            except ValueError:
                pass
            else:
                del tokens[dx_index]
                for i, token in enumerate(tokens):
                    if token.startswith("f/"):
                        tokens.insert(i + 1, "DX")
                        break
            tokens[0] = "Nikkor"
            self.model_en = " ".join(tokens)

    def read_first_entry(self):
        """Extracts metadata from the first calibration entry in the file and
        populates the object instance.  This refers to the lens model and
        maker, the camera maker, the cropfactor used for the calibration, and
        some other things.  It populates most of this instance's fields.
        """
        entry = self.calibration_entries[0]
        camera_make = self.read_field(entry, "Make")
        self.camera_model = self.read_field(entry, "Model", camera_make)
        camera = lensfun_cameras.get((camera_make.lower(), self.camera_model.lower()))
        self.model_en = self.read_field(entry, "LensPrettyName")
        if self.read_field(entry, "CameraRawProfile", "True").lower() == "false":
            self.model_en += " (JPEGs)"
            self.raw = False
        else:
            self.raw = True
        self.maker = self.clean_lens_maker(self.model_en)
        try:
            self.model = self.read_field(entry, "Lens")
        except FieldNotFoundError:
            self.model = self.model_en
        if unusable_lens_name_regex.match(self.model):
            self.model = self.model_en
        self.model = LensfunLens.sanitize_lcp_lens_model_name(self.model)
        self.model_en = LensfunLens.sanitize_lcp_lens_model_name(self.model_en)
        self.make_model_en_prettier()
        try:
            self.cropfactor = float(self.read_field(entry, "SensorFormatFactor"))
        except FieldNotFoundError:
            self.cropfactor = camera and camera.cropfactor or 1
        self.fixed_lens_mount = None
        if camera:
            if camera.mount[0].islower():
                self.fixed_lens_mount = camera.mount
        elif re.search(r"coolpix|finefix|powershot|sony dsc|cyber-?shot|apple|iphone|hero|inspire|cgo2gb|samsung ex1",
                       self.model_en, re.IGNORECASE):
            self.fixed_lens_mount = "compactCamera" + str(abs(hash(self.model_en)))
            camera_element = ElementTree.Element("camera")
            ElementTree.SubElement(camera_element, "maker").text = camera_make
            ElementTree.SubElement(camera_element, "model").text = self.camera_model
            ElementTree.SubElement(camera_element, "mount").text = self.fixed_lens_mount
            ElementTree.SubElement(camera_element, "cropfactor").text = str(self.cropfactor)
            self.compact_cameras.append(camera_element)
        self.lensfun_lens = None
        models = [self.model] if self.model == self.model_en else [self.model, self.model_en]
        best_scores = (-1000, 0)
        for lens in lensfun_lenses:
            scores = lens.matches(models, self.cropfactor, self.fixed_lens_mount)
            if scores > best_scores:
                best_scores = scores
                self.lensfun_lens = lens
        if self.lensfun_lens:
            self.maker = child_without_attributes(self.lensfun_lens.element, "maker").text
            self.model = child_without_attributes(self.lensfun_lens.element, "model").text

    def fix_focal_lengths(self):
        """Assures that every LCP file calibration entry has a focal length.  Contrary
        to the official Adobe specs, many calibration entries lack a focal
        length.  In this routine, we try to reconstruct it in those cases from
        the lens model name.  If this fails, the respective entry is removed.
        """
        clean_entries = []
        for entry in self.calibration_entries:
            try:
                self.read_field(entry, "FocalLength")
            except FieldNotFoundError:
                match = focal_length_regex.search(self.model)
                focal_length = None
                if match:
                    focal_length = match.group(1)
                else:
                    try:
                        match = focal_length_regex.search(self.read_field(entry, "Lens"))
                    except FieldNotFoundError:
                        if "inspire 1 fc350" in self.model.lower():
                            focal_length = 20.7
                    else:
                        if match:
                            focal_length = match.group(1)
                if focal_length:
                    if self.old_format:
                        ElementTree.SubElement(entry, camera_ns + "FocalLength").text = focal_length
                        clean_entries.append(entry)
                    else:
                        entry.attrib[camera_ns + "FocalLength"] = focal_length
                        clean_entries.append(entry)
                else:
                    print("Warning: No focal length could be determined for {} / {}.".format(self.maker, self.model))
                    raise NoLCPDataUsed
            else:
                clean_entries.append(entry)
        self.calibration_entries = clean_entries

    def best_entries(self, tca):
        """Returns the entries best suited for distortion and TCA data, because Lensfun
        can only use one per focal length.

        :param tca: Whether we look for TCA entries.  If ``False``, we look for
          distortion entries.

        :type tca: bool

        :return:
          dictionary mapping the focal length to the best entry for that focal
          length

        :rtype: dict mapping float to ElementTree.Element
        """
        current_distances, current_apertures, result = {}, {}, {}
        for entry in self.calibration_entries:
            perspective_entry = entry.find(camera_ns + "PerspectiveModel")
            if perspective_entry is None:
                perspective_entry = entry.find(camera_ns + "FisheyeModel")
                if perspective_entry is None:
                    continue
            if tca and self.get_description_element_maybe(perspective_entry). \
                    find(camera_ns + "ChromaticRedGreenModel") is None:
                continue
            focal_length = float(self.read_field(entry, "FocalLength"))
            current_distance = current_distances.setdefault(focal_length, -2)
            current_aperture = current_apertures.setdefault(focal_length, 1001)
            distance = float(self.read_field(entry, "FocusDistance", -1))
            if distance > current_distance:
                aperture = float(self.read_field(entry, "ApertureValue", 1000))
                if abs(aperture - 8) < abs(current_aperture - 8):
                    current_distances[focal_length] = distance
                    current_apertures[focal_length] = aperture
                    result[focal_length] = entry
        return result

    def generate_distortion_entries(self):
        """Generates distortion entries from the LCP file.  The entries have the
        Lensfun format and can be included as children into a ``<calibration>``
        tag.  They are sorted by increasing focal length.  The information
        whether the distortion data refers to Adobe's fisheye model is
        returned, too.

        Curiously enough, Adobe's fisheye model can be realised easily in
        Lensfun by simply adding the ``<distortion>`` tag with the ``k1`` and
        ``k2`` parameters, and setting ``<type>`` to ``fisheye``.

        :return:
          the ``<distortion>`` elements for the Lensfun output, and whether
          it's a fisheye lens

        :rtype: list of ElementTree.Element, bool
        """
        best_entries = self.best_entries(tca=False)
        elements = []
        fisheye = None
        for focal_length in sorted(best_entries):
            entry = best_entries[focal_length].find(camera_ns + "PerspectiveModel")
            if entry is None:
                entry = best_entries[focal_length].find(camera_ns + "FisheyeModel")
                assert fisheye != False
                fisheye = True
            else:
                assert fisheye != True
                fisheye = False
            entry = self.get_description_element_maybe(entry)
            element = ElementTree.Element("distortion", {"focal": str(focal_length), "model": "acm"})
            for i in range(1, 6):
                k = self.read_field(entry, "RadialDistortParam{}".format(i), None)
                if k is not None:
                    element.attrib["k{}".format(i)] = k
            elements.append(element)
        return elements, fisheye

    def generate_tca_entries(self):
        """Generates TCA entries from the LCP file.  The entries have the Lensfun
        format and can be included as children into a ``<calibration>`` tag.
        They are sorted by increasing focal length.

        :return:
          the ``<tca>`` elements for the Lensfun output

        :rtype: list of ElementTree.Element
        """
        best_entries = self.best_entries(tca=True)
        elements = []
        for focal_length in sorted(best_entries):
            entry = best_entries[focal_length].find(camera_ns + "PerspectiveModel")
            if entry is None:
                entry = best_entries[focal_length].find(camera_ns + "FisheyeModel")
            entry = self.get_description_element_maybe(entry)
            element = ElementTree.Element("tca", {"focal": str(focal_length), "model": "acm"})
            for type_ in ["alpha", "beta"]:
                chromatic_element = entry.find(camera_ns + ("ChromaticRedGreenModel" if type_ == "alpha" else
                                                            "ChromaticBlueGreenModel"))
                scale_factor = self.read_field(chromatic_element, "ScaleFactor", None)
                if scale_factor is not None:
                    element.attrib[type_ + "0"] = scale_factor
                for i in range(1, 6):
                    parameter = self.read_field(chromatic_element, "RadialDistortParam{}".format(i), None)
                    if parameter is not None:
                        element.attrib[type_ + str(i)] = parameter
            elements.append(element)
        return elements

    def generate_vignetting_entries(self):
        """Generates vignetting entries from the LCP file.  The entries have the
        Lensfun format and can be included as children into a ``<calibration>``
        tag.  They are sorted by increasing focal length, then by increasing
        f-stop number, and then by increasing distance.

        :return:
          the ``<vignetting>`` elements for the Lensfun output

        :rtype: list of ElementTree.Element
        """
        match = re.search(r"f/?(?P<min>[0-9.]+)", self.model, re.IGNORECASE)
        if match:
            aperture_min = float(match.group("min"))
        else:
            aperture_min = 0
        elements = []
        def sort_key(entry):
            focal_length = float(self.read_field(entry, "FocalLength"))
            aperture = float(self.read_field(entry, "ApertureValue", "nan"))
            distance = float(self.read_field(entry, "FocusDistance", "nan"))
            return (focal_length, aperture, distance)
        for entry in sorted(self.calibration_entries, key=sort_key):
            focal_length = self.read_field(entry, "FocalLength")
            try:
                aperture = self.read_field(entry, "ApertureValue")
                distance = self.read_field(entry, "FocusDistance")
            except FieldNotFoundError:
                continue
            if aperture_min > float(aperture):
                continue
            entry = entry.find(camera_ns + "PerspectiveModel")
            if entry is not None:
                entry = self.get_description_element_maybe(entry)
            if entry is not None:
                entry = entry.find(camera_ns + "VignetteModel")
                if entry is not None:
                    entry = self.get_description_element_maybe(entry)
                    element = ElementTree.Element("vignetting", {"focal": focal_length, "model": "acm", "aperture": aperture,
                                                                 "distance": distance})
                    for i in range(1, 6):
                        α = self.read_field(entry, "VignetteModelParam{}".format(i), None)
                        if α is not None:
                            element.attrib["alpha{}".format(i)] = α
                    elements.append(element)
        return elements

    def guess_ilc_mounts(self):
        """Guess the list of available mounts of this lens.  This routine gives
        senseful results only if the lens is for interchangeable lens cameras
        (rather than compact cameras).  First and foremost, it uses the lens
        maker for its guesswork.

        :return:
          the mounts for which this lens is probably available, as a list of
          ``<mount>`` elements

        :rtype: list of ElementTree.Element
        """
        mounts = set()
        if self.maker == "Nikon":
            if 2.6 < self.cropfactor < 2.8:
                mounts.add("Nikon CX")
            else:
                mounts.add("Nikon F AF")
        elif self.maker == "Canon":
            if "ef-m" in self.model.lower():
                mounts.add("Canon EF-M")
            else:
                mounts.add("Canon EF")
        elif self.maker == "Sony":
            if "E " in self.model:
                mounts.add("Sony E")
            else:
                mounts.add("Sony Alpha")
        elif self.maker == "Pentax":
            if "645" in self.model:
                mounts.add("Mamiya 645")
            else:
                mounts.add("Pentax KAF")
        elif self.maker == "Sigma":
            if "E " in self.model:
                mounts.add("Sony E")
            else:
                mounts.update({"Sigma SA", "Nikon F AF", "Sony Alpha", "Pentax KAF", "Canon EF", "Minolta AF", "Canon FD",
                               "Olympus OM", "4/3 System"})
        elif self.maker == "Zeiss":
            if "E " in self.model:
                mounts.add("Sony E")
            else:
                mounts.update({"Nikon F AF", "Fujifilm X", "Canon EF", "Leica M"})
        elif self.maker == "Voigtländer":
            mounts.update({"DKL", "Leica M", "M42", "Micro 4/3 System", "Nikon F"})
        elif self.maker == "Leica":
            if "-S " in self.model or " S " in self.model:
                mounts.add("Leica S")
            elif "-R " in self.model or " R " in self.model:
                mounts.add("Leica R")
            else:
                mounts.add("Leica M")
        elif self.maker == "Mamiya":
            mounts.add("Mamiya 645")
        elif self.maker == "Tokina":
            mounts.update({"Nikon F AF", "Sony Alpha", "Pentax KAF", "Canon EF", "Canon FD", "Minolta M", "Olympus OM"})
        elif self.maker == "Tamron":
            mounts.update({"Nikon F AF", "Sony Alpha", "Pentax KAF", "Canon EF"})
        elif self.maker == "Samsung":
            if "NX" in self.model or "NX" in self.camera_model:
                mounts.add("Samsung NX")
        elif self.maker == "HandeVision":
            if "E " in self.model:
                mounts.add("Sony E")
            else:
                mounts.update({"Canon EF-M", "Micro 4/3 System", "Fujifilm X"})
        elif self.maker == "SLR Magic":
            mounts.update({"Leica M", "Sony E", "Micro 4/3 System", "Fujifilm X"})
        elif self.maker == "Hasselblad":
            if "LF" in self.model:
                mounts.add("Sony E")
            else:
                mounts.add("Hasselblad H")
        elif self.maker == "Mitakon":
            mounts.update({"Sony E", "Micro 4/3 System", "Fujifilm X", "Canon EF", "Nikon F AI"})
        elif self.maker == "Venus Optics":
            mounts.update({"Nikon F AF", "Sony Alpha", "Pentax KAF", "Canon EF"})
        elif self.maker == "Schneider-Kreuznach":
            if "phase one" in self.camera_model.lower():
                mounts.update({"Hasselblad H", "Mamiya 645"})
        elif self.maker == "Phase One":
            mounts.update({"Hasselblad H", "Mamiya 645"})
        elif self.maker == "Lomography":
            mounts.update({"Nikon F AF", "Canon EF"})
        if not mounts:
            print("Warning: Fall back to default mounts for {} / {}.".format(self.maker, self.model))
            mounts = {"Sigma SA", "Nikon F AF", "Sony Alpha", "Pentax KAF", "Canon EF", "Minolta AF", "Canon FD",
                      "Olympus OM", "4/3 System"}
        result = []
        for mount in mounts:
            element = ElementTree.Element("mount")
            element.text = mount
            result.append(element)
        return result

    def xml_element(self):
        """Generates the Lensfun XML entry for this lens.  Here, everything
        comes together.

        :return:
          the ``<lens>`` element with the complete data for this lens, ready to
          be send into the final output

        :rtype: ElementTree.Element

        :raises NoLCPDataUsed: if no data from the LCP file really made it into
          the final ``<lens>`` entry, so its inclusion is pointless.
        """
        if self.lensfun_lens:
            # Re-populating an existing Lensfun entry.
            lens = copy.deepcopy(self.lensfun_lens.element)
            calibration = lens.find("calibration")
            if calibration is None:
                calibration = ElementTree.SubElement(lens, "calibration")
            lcp_entries_used = False
            if calibration.find("distortion") is None or args.prefer_lcp:
                entries, fisheye = self.generate_distortion_entries()
                if entries:
                    type_ = lens.find("type")
                    if fisheye:
                        if type_ is not None:
                            type_.text = "fisheye"
                        else:
                            type_element = ElementTree.Element("type")
                            type_element.text = "fisheye"
                            lens.insert(max(len(lens) - 1, 0), type_element)
                    elif type_ is not None:
                        lens.remove(type_)
                    for entry in calibration.findall("distortion"):
                        calibration.remove(entry)
                    calibration.extend(entries)
                    lcp_entries_used = True
            if calibration.find("tca") is None or args.prefer_lcp:
                entries = self.generate_tca_entries()
                if entries:
                    for entry in calibration.findall("tca"):
                        calibration.remove(entry)
                    calibration.extend(entries)
                    lcp_entries_used = True
            if calibration.find("vignetting") is None or args.prefer_lcp:
                entries = self.generate_vignetting_entries()
                if entries:
                    for entry in calibration.findall("vignetting"):
                        calibration.remove(entry)
                    calibration.extend(entries)
                    lcp_entries_used = True
            if not lcp_entries_used:
                raise NoLCPDataUsed
        else:
            # Creating a <lens> element from scratch.
            lens = ElementTree.Element("lens")
            ElementTree.SubElement(lens, "maker").text = self.maker
            ElementTree.SubElement(lens, "model").text = self.model
            if self.fixed_lens_mount:
                suffix = "" if self.raw else " (JPEGs)"
                ElementTree.SubElement(lens, "model", lang="en").text = "fixed lens" + suffix
                ElementTree.SubElement(lens, "model", lang="de").text = "festes Objektiv" + suffix
                ElementTree.SubElement(lens, "mount").text = self.fixed_lens_mount
            else:
                if self.model != self.model_en:
                    ElementTree.SubElement(lens, "model", {"lang": "en"}).text = self.model_en
                lens.extend(self.guess_ilc_mounts())
            ElementTree.SubElement(lens, "cropfactor").text = str(self.cropfactor)
            entries, fisheye = self.generate_distortion_entries()
            if fisheye:
                ElementTree.SubElement(lens, "type").text = "fisheye"
            calibration = ElementTree.SubElement(lens, "calibration")
            if entries:
                calibration.extend(entries)
            entries = self.generate_tca_entries()
            if entries:
                calibration.extend(entries)
            entries = self.generate_vignetting_entries()
            if entries:
                calibration.extend(entries)
        self.correction_coverage = 0
        if calibration.find("distortion") is not None:
            self.correction_coverage += 1
        if calibration.find("tca") is not None:
            self.correction_coverage += 1
        if calibration.find("vignetting") is not None:
            self.correction_coverage += 1
        if not self.correction_coverage:
            print("Warning: LCP file for {} / {} contained no correction data.".format(self.maker, self.model))
            raise NoLCPDataUsed
        return lens

    @property
    def normalized_cropfactor(self):
        """Returns the normalized cropfactor for this lens.  This is the cropfactor
        rounded to one decimal place.  In addition, APS-C cropfactors are all
        clamped to exactly 1.5 (1.6 for Canon), and APS-H is clamped to 1.3.
        This way, duplicates in output can be reduced.

        :return:
          the normalised cropfactor

        :rtype: float
        """
        if 1.5 <= self.cropfactor <= 1.57:
            return 1.5
        elif 1.58 <= self.cropfactor <= 1.63:
            return 1.6
        elif 1.2 <= self.cropfactor <= 1.3:
            return 1.3
        else:
            return round(self.cropfactor * 10) / 10


def read_lcp_file(filepath):
    """Reads and parses a single LCP file.  This must be a top-level function in
    order to work together with the multiprocessing module.

    :param filepath: full path to the LCP file

    :type filepath: str

    :return:
      The `LCPLens` object, or ``None`` if the file doesn't contain usable
      data.  This needn't mean that the file itself had issues, but e.g. that a
      Lensfun DB entry shadows all data in the LCP file.

    :rtype: `LCPLens` or ``NoneType``
    """
    try:
        return LCPLens(filepath)
    except (ElementTree.ParseError, NoLCPDataUsed):
        return None

def read_lcp_files():
    """Returns all LCP lenses (aka LCP files) that actually make it into the final
    output.

    :return:
      all LCP lenses for the output XML file

    :rtype: set of `LCPLens`
    """
    results = set()
    for root, __, filenames in os.walk(args.input_directory):
        for filename in filenames:
            filepath = os.path.join(root, filename)
            results.add(read_lcp_file(filepath))

    lcp_lenses_dict = {}
    for lcp_lens in results:
        if lcp_lens is None:
            continue
        key = (lcp_lens.maker, lcp_lens.model, lcp_lens.normalized_cropfactor)
        already_existing_lens = lcp_lenses_dict.get(key)
        if not already_existing_lens or already_existing_lens.correction_coverage < lcp_lens.correction_coverage \
                or already_existing_lens.correction_coverage == lcp_lens.correction_coverage and \
                already_existing_lens.cropfactor > lcp_lens.cropfactor:
            lcp_lenses_dict[key] = lcp_lens

    # pool = multiprocessing.Pool()
    # results = set()
    # for root, __, filenames in os.walk(args.input_directory):
    #     for filename in filenames:
    #         filepath = os.path.join(root, filename)
    #         results.add(pool.apply_async(read_lcp_file, (filepath,)))
    # pool.close()
    # pool.join()
    # lcp_lenses_dict = {}
    # for result in results:
    #     lcp_lens = result.get()
    #     if lcp_lens:
    #         key = (lcp_lens.maker, lcp_lens.model, lcp_lens.normalized_cropfactor)
    #         already_existing_lens = lcp_lenses_dict.get(key)
    #         if not already_existing_lens or already_existing_lens.correction_coverage < lcp_lens.correction_coverage \
    #                 or already_existing_lens.correction_coverage == lcp_lens.correction_coverage and \
    #                 already_existing_lens.cropfactor > lcp_lens.cropfactor:
    #             lcp_lenses_dict[key] = lcp_lens

    return set(lcp_lenses_dict.values())

lcp_lenses = read_lcp_files()


# Part III
#
# Writing the output to an XML file


output = ElementTree.Element("lensdatabase", {"version": "2"})
for camera in LCPLens.compact_cameras:
    output.append(camera)
for lens in lcp_lenses:
    output.append(lens.xml_element)
indent(output)
if len(output):
    output.text = "\n\n    "
    output.tail = "\n"
    output[-1].tail = "\n\n"
with open(args.output, "w") as f:
    f.write(ElementTree.tostring(output, encoding="unicode"))
