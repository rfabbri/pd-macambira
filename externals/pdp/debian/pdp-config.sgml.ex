<!doctype refentry PUBLIC "-//OASIS//DTD DocBook V4.1//EN" [

<!-- Process this file with docbook-to-man to generate an nroff manual
     page: `docbook-to-man manpage.sgml > manpage.1'.  You may view
     the manual page with: `docbook-to-man manpage.sgml | nroff -man |
     less'.  A typical entry in a Makefile or Makefile.am is:

manpage.1: manpage.sgml
	docbook-to-man $< > $@

    
	The docbook-to-man binary is found in the docbook-to-man package.
	Please remember that if you create the nroff version in one of the
	debian/rules file targets (such as build), you will need to include
	docbook-to-man in your Build-Depends control field.

  -->

  <!-- Fill in your name for FIRSTNAME and SURNAME. -->
  <!ENTITY dhfirstname "<firstname>Pablo</firstname>">
  <!ENTITY dhsurname   "<surname>Martín</surname>">
  <!-- Please adjust the date whenever revising the manpage. -->
  <!ENTITY dhdate      "<date>julio 10, 2003</date>">
  <!-- SECTION should be 1-8, maybe w/ subsection other parameters are
       allowed: see man(7), man(1). -->
  <!ENTITY dhsection   "<manvolnum>1</manvolnum>">
  <!ENTITY dhemail     "<email>caedes@sindominio.net</email>">
  <!ENTITY dhusername  "Pablo Martín">
  <!ENTITY dhucpackage "<refentrytitle>PDP</refentrytitle>">
  <!ENTITY dhpackage   "pdp">

  <!ENTITY debian      "<productname>Debian</productname>">
  <!ENTITY gnu         "<acronym>GNU</acronym>">
  <!ENTITY gpl         "&gnu; <acronym>GPL</acronym>">
]>

<refentry>
  <refentryinfo>
    <address>
      &dhemail;
    </address>
    <author>
      &dhfirstname;
      &dhsurname;
    </author>
    <copyright>
      <year>2003</year>
      <holder>&dhusername;</holder>
    </copyright>
    &dhdate;
  </refentryinfo>
  <refmeta>
    &dhucpackage;

    &dhsection;
  </refmeta>
  <refnamediv>
    <refname>&dhpackage;</refname>

    <refpurpose>script to get information about the installed version of PDP</refpurpose>
  </refnamediv>
  <refsynopsisdiv>
    <cmdsynopsis>
      <command>&dhpackage;</command>

      <arg><option>--version</option></arg>

      <arg><option>--cflags</option></arg>
      <arg><option>--libdir</option></arg>
    </cmdsynopsis>
  </refsynopsisdiv>
  <refsect1>
    <title>DESCRIPTION</title>

    <para>
      <command>&dhpackage;</command> is a tool that is used to configure to determine the compiler and linker flags that should be used to compile and links modules that use PDP.
     </para>

  </refsect1>
  <refsect1>
    <title>OPTIONS</title>

    <para><command>&dhpackage;</command> accepts the following options:</para>

    <variablelist>
      <varlistentry>
        <term><option>--version</option>
        </term>
        <listitem>
          <para>Print the currently installed version of PDP on the standard output.</para>
        </listitem>
      </varlistentry>
      <varlistentry>
        <term><option>--cflags</option>
        </term>
        <listitem>
          <para>Print the compiler flags that are necessary to compile a PDP module.</para>
        </listitem>
      </varlistentry>
      <varlistentry>
        <term><option>--libdir</option>
        </term>
        <listitem>
          <para>Print the linker flags that are necessary to link a PDP module.</para>
        </listitem>
      </varlistentry>
    </variablelist>
  </refsect1>
  <refsect1>
    <title>SEE ALSO</title>

    <para>pd (1).</para>
  </refsect1>
  <refsect1>
    <title>AUTHOR</title>

    <para>This manual page was written by &dhusername; &dhemail; for
      the &debian; system (but may be used by others).  Permission is
      granted to copy, distribute and/or modify this document under
      the terms of the &gnu; Free Documentation
      License, Version 1.1 or any later version published by the Free
      Software Foundation; with no Invariant Sections, no Front-Cover
      Texts and no Back-Cover Texts.</para>

  </refsect1>
</refentry>

<!-- Keep this comment at the end of the file
Local variables:
mode: sgml
sgml-omittag:t
sgml-shorttag:t
sgml-minimize-attributes:nil
sgml-always-quote-attributes:t
sgml-indent-step:2
sgml-indent-data:t
sgml-parent-document:nil
sgml-default-dtd-file:nil
sgml-exposed-tags:nil
sgml-local-catalogs:nil
sgml-local-ecat-files:nil
End:
-->


