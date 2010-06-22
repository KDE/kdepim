<?xml version="1.0" encoding="utf-8"?>
<kcfg xmlns="http://www.kde.org/standards/kcfg/1.0"
xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
xsi:schemaLocation="http://www.kde.org/standards/kcfg/1.0 http://www.kde.org/standards/kcfg/1.0/kcfg.xsd">

  <include>kglobalsettings.h</include>
  <kcfgfile name="mailcomposerrc" />
  <group name="General">
    <entry  name="DefaultDomain" type="String" key="Default domain">
      <default></default>
    </entry>
  </group>

  <group name="Composer">

    <entry name="ReplyPrefixes" type="StringList" key="reply-prefixes">
      <default>Re\\s*:,Re\\[\\d+\\]:,Re\\d+:</default>
    </entry>

    <entry name="ReplaceReplyPrefix" type="Bool" key="replace-reply-prefix">
      <label>Replace recognized prefi&amp;x with "Re:"</label>
        <default>true</default>
    </entry>

    <entry name="ForwardPrefixes" type="StringList" key="forward-prefixes">
      <default>Fwd:,FW:</default>
    </entry>

    <entry name="ReplaceForwardPrefix" type="Bool" key="replace-forward-prefix">
      <label>Replace recognized prefix with "&amp;Fwd:"</label>
      <default>true</default>
    </entry>

    <entry name="CustomMsgIDSuffix" type="String" key="myMessageIdSuffix">
      <default></default>
    </entry>

    <entry name="UseCustomMessageIdSuffix" type="bool" key="useCustomMessageIdSuffix">
      <default>false</default>
    </entry>

    <entry name="QuoteSelectionOnly" type="Bool">
      <label>Only quote selected text when replying</label>
      <default>true</default>
    </entry>

    <entry name="ForceReplyCharset" type="Bool" key="force-reply-charset">
      <label>Keep original charset when replying or forwarding if possible</label>
      <default>false</default>
    </entry>

    <entry name="PreferredCharsets" key="pref-charsets" type="StringList">
        <default>us-ascii,iso-8859-1,locale,utf-8</default>
    </entry>

    <entry name="AutoTextSignature" type="String" key="signature">
      <label>A&amp;utomatically insert signature</label>
      <default>auto</default>
    </entry>
    <entry name="PrependSignature" type="Bool" key="prepend-signature">
      <label>Insert signature above quoted text</label>
      <default>false</default>
    </entry>
    <entry name="DashDashSignature" type="Bool" key="dash-dash-signature">
      <label>Prepend separator to signature</label>
      <default>true</default>
    </entry>

    <entry name="AllowSemicolonAsAddressSeparator" type="Bool">
      <default>${ALLOW_SEMICOLON_AS_ADDRESS_SEPARATOR_DEFAULT}</default>
      <label>Allow the semicolon character (';') to be used as separator in the message composer.</label>
    </entry>
    <entry name="ShowRecentAddressesInComposer" type="Bool" key="showRecentAddressesInComposer">
      <label>Use recent addresses for autocompletion</label>
      <whatsthis>Disable this option if you do not want recently used addresses to appear in the autocompletion list in the composer's address fields.</whatsthis>
      <default>true</default>
    </entry>

    <entry name="MaximumRecipients" type="Int">
      <label>Maximum number of recipient editor lines.</label>
      <default>200</default>
    </entry>
    <entry name="SecondRecipientTypeDefault" type="Enum">
      <choices>
        <choice name="To"/>
        <choice name="Cc"/>
      </choices>
      <default>To</default>
    </entry>
    <entry name="ShowMessagePartDialogOnAttach" type="Bool" key="showMessagePartDialogOnAttach">
      <default>false</default>
    </entry>

    <entry name="MaximumAttachmentSize" type="Int">
        <label>The maximum size in MB that email attachments are allowed to have (-1 for no limit).</label>
        <default>-1</default>
    </entry>
    <entry name="OutlookCompatibleAttachments" type="Bool" key="outlook-compatible-attachments">
      <label>Outlook-compatible attachment naming</label>
      <whatsthis>Turn this option on to make Outlook &#8482; understand attachment names containing non-English characters</whatsthis>
      <default>false</default>
    </entry>


  </group>

  <group name="sending mail">
    <entry name="SendImmediate" type="Bool" key="Immediate">
      <default>true</default>
    </entry>
  </group>

</kcfg>
