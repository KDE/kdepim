<?xml version="1.0" encoding="utf-8"?>
<kcfg xmlns="http://www.kde.org/standards/kcfg/1.0"
xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
xsi:schemaLocation="http://www.kde.org/standards/kcfg/1.0 http://www.kde.org/standards/kcfg/1.0/kcfg.xsd">

  <include>kglobalsettings.h</include>
  <include>kcolorscheme.h</include>
  <kcfgfile name="mailviewerrc" />

  <group name="Fonts">
    <entry name="FixedFont" type="Font" key="fixed-font">
      <default code="true">KGlobalSettings::fixedFont()</default>
    </entry>
    <entry name="MinimumFontSize" type="Int">
      <label>When we render html do not use font size inferior to minimum size.</label>
      <default>8</default>
    </entry>
  </group>

  <group name="Reader">
    <entry name="ShowToltecReplacementText" type="Bool">
      <label>When encountering a Toltec scheduling message, display a custom replacement text for it.</label>
      <default>true</default>
    </entry>
    <entry name="ToltecReplacementText" type="String">
      <label>The text that will be displayed as a replacement when encountering Toltec scheduling messages.</label>
      <default code="true">MessageViewer::ObjectTreeParser::defaultToltecReplacementText()</default>
    </entry>
    <entry name="AutoImportKeys" type="Bool">
      <default>false</default>
    </entry>
    <entry name="StoreDisplayedMessagesUnencrypted" type="Bool" key="store-displayed-messages-unencrypted">
      <default>false</default>
    </entry>
    <entry name="showColorBar" type="Bool">
      <default>false</default>
      <label>Show HTML status bar</label>
    </entry>
    <entry name="showSpamStatus" type="Bool">
      <default>true</default>
      <label>Show spam status in fancy headers</label>
    </entry>
    <entry name="ShowEmoticons" type="Bool">
      <default>true</default>
      <label>Replace smileys by emoticons</label>
      <whatsthis>Enable this if you want smileys like :-) appearing in the message text to be replaced by emoticons (small pictures).</whatsthis>
    </entry>
    <entry name="ShowExpandQuotesMark" type="Bool">
      <default>false</default>
      <label>Show expand/collapse quote marks</label>
      <whatsthis>Enable this option to show different levels of quoted text. Disable to hide the levels of quoted text.</whatsthis>
    </entry>
    <entry name="CollapseQuoteLevelSpin" type="Int">
      <label>Automatic collapse level:</label>
      <default>3</default>
      <min>0</min>
      <max>10</max>
    </entry>
    <entry name="ShrinkQuotes" type="Bool">
      <default>false</default>
      <label>Reduce font size for quoted text</label>
      <whatsthis>Enable this option to show quoted text with a smaller font.</whatsthis>
    </entry>
    <entry name="ChiasmusDecryptionKey" type="String"></entry>
    <entry name="ChiasmusDecryptionOptions" type="String"></entry>
    <entry name="ShowUserAgent" type="Bool">
      <default>false</default>
      <label>Show user agent in fancy headers</label>
      <whatsthis>Enable this option to get the User-Agent and X-Mailer header lines displayed when using fancy headers.</whatsthis>
    </entry>
    <entry name="AllowAttachmentDeletion" type="Bool">
      <default>true</default>
      <label>Allow to delete attachments of existing mails.</label>
    </entry>
    <entry name="AllowAttachmentEditing" type="Bool">
      <default>false</default>
      <label>Allow to edit attachments of existing mails.</label>
    </entry>
    <entry name="AlwaysDecrypt" type="Bool">
      <default>false</default>
      <label>Always decrypt messages when viewing or ask before decrypting</label>
    </entry>
    <entry name="MimeTreeLocation" type="Enum">
      <label>Message Structure Viewer Placement</label>
      <choices>
        <choice name="top">
          <label>Above the message pane</label>
        </choice>
        <choice name="bottom">
          <label>Below the message pane</label>
        </choice>
      </choices>
      <default>bottom</default>
    </entry>
    <entry name="MimeTreeMode" type="Enum">
      <label>Message Structure Viewer</label>
      <choices>
        <choice name="Never">
          <label>Show never</label>
        </choice>
        <choice name="Always">
          <label>Show always</label>
        </choice>
      </choices>
      <default>Never</default>
    </entry>
    <entry name="numberOfAddressesToShow" type="Int">
      <label>Number of addresses to show before collapsing</label>
      <default>4</default>
      <min>1</min>
    </entry>
    <entry name="MimePaneHeight" type="Int">
      <default>100</default>
    </entry>
    <entry name="MessagePaneHeight" type="Int">
      <default>180</default>
    </entry>
    <entry name="headerStyle" type="String" key="header-style">
      <label>What style of headers should be displayed</label>
      <default>fancy</default>
    </entry>
    <entry name="headerSetDisplayed" type="String" key="header-set-displayed">
      <label>How much of headers should be displayed</label>
      <default>rich</default>
    </entry>
     <entry name="htmlMail" type="Bool">
       <label>Prefer HTML to plain text</label>
       <default>false</default>
    </entry>
    <entry name="htmlLoadExternal" type="Bool">
      <label>Allow messages to load external references from the Internet</label>
      <default>false</default>
    </entry>
    <entry name="zoomTextOnly" type="Bool">
      <label>Zoom only text</label>
      <default>false</default>
    </entry>
    <entry name="attachmentStrategy" type="String" key="attachment-strategy">
      <label>How attachments are shown</label>
      <default>Smart</default>
    </entry>
    <entry name="RecycleQuoteColors" type="Bool">
      <label>Specifies whether to reuse the quote color, beyond the 3rd level</label>
      <default>false</default>
    </entry>
    <entry name="AccessKeyEnabled" type="Bool">
      <label>Activate Access Key</label>
      <default>true</default>
    </entry>
  </group>

  <group name="TextIndex">
    <entry name="automaticDecrypt" type="Bool"
    key="automaticDecrypt">
      <default>true</default>
    </entry>
  </group>

  <group name="MDN">
    <entry name="notSendWhenEncrypted" type="Bool" key="not-send-when-encrypted">
      <label>Do not send MDNs in response to encrypted messages</label>
      <default>true</default>
    </entry>
    <entry name="DefaultPolicy" type="Int" key="default-policy">
      <label>Specifies the default policy to use, for the Message Disposition Notifications (for internal use only)</label>
      <default>0</default>
    </entry>
    <entry name="QuoteMessage" type="Int" key="quote-message">
      <label>Specifies the default quoting action to take, when replying to a message (for internal use only)</label>
      <default>0</default>
    </entry>
  </group>

  <group name="Behaviour">
    <entry name="DelayedMarkAsRead"  type="Bool">
      <default>true</default>
    </entry>
    <entry name="DelayedMarkTime"  type="UInt">
      <default>0</default>
    </entry>
  </group>

    <group name="Invitations">
      <entry name="LegacyMangleFromToHeaders" type="Bool">
        <label>Mangle From:/To: headers in replies to replies</label>
        <whatsthis>Microsoft Outlook has a number of shortcomings in its implementation of the iCalendar standard; this option works around one of them. If you have problems with Outlook users not being able to get your replies, try setting this option.</whatsthis>
        <default>${LEGACY_MANGLE_FROM_TO_HEADERS}</default>
      </entry>

      <entry name="LegacyBodyInvites" type="Bool">
        <label>Send groupware invitations in the mail body</label>
        <whatsthis>Microsoft Outlook has a number of shortcomings in its implementation of the iCalendar standard; this option works around one of them. If you have problems with Outlook users not being able to get your invitations, try setting this option.</whatsthis>
        <default>${LEGACY_BODY_INVITES}</default>
      </entry>

     <entry name="ExchangeCompatibleInvitations" type="Bool">
        <label>Exchange-compatible invitation naming</label>
        <whatsthis>Microsoft Outlook, when used in combination with a Microsoft Exchange server, has a problem understanding standards-compliant groupware email. Turn this option on to send groupware invitations in a way that Microsoft Exchange understands.</whatsthis>
        <default>${EXCHANGE_COMPATIBLE_INVITATIONS}</default>
      </entry>

      <entry name="OutlookCompatibleInvitationReplyComments" type="Bool">
        <label>Outlook compatible invitation reply comments</label>
        <whatsthis>When replying to invitations, send the reply comment in way that Microsoft Outlook understands.</whatsthis>
        <default>${OUTLOOK_COMPATIBLE_INVITATION_REPLY_COMMENTS}</default>
      </entry>

      <entry name="OutlookCompatibleInvitationComparisons" type="Bool">
        <label>Show invitation update differences in the Outlook style</label>
        <whatsthis>When viewing invitation updates, show the differences in the Microsoft Outlook style.</whatsthis>
        <default>true</default>
      </entry>

      <entry name="AutomaticSending" type="Bool">
        <label>Automatic invitation sending</label>
        <whatsthis>When this is checked, you will not see the mail composer window. Instead, all invitation mails are sent automatically. If you want to see the mail before sending it, you can uncheck this option. However, be aware that the text in the composer window is in iCalendar syntax, and you should not try modifying it by hand.</whatsthis>
        <default>true</default>
      </entry>

      <entry name="DeleteInvitationEmailsAfterSendingReply" type="Bool">
        <label>Delete invitation emails after the reply to them has been sent</label>
        <whatsthis>When this is checked, received invitation emails that have been replied to will be moved to the Trash folder, once the reply has been successfully sent.</whatsthis>
        <default>true</default>
      </entry>

      <entry name="AskForCommentWhenReactingToInvitation"  type="Enum">
        <label></label>
        <whatsthis></whatsthis>
        <choices>
          <choice name="NeverAsk"/>
          <choice name="AskForAllButAcceptance"/>
          <choice name="AlwaysAsk"/>
        </choices>
        <default>AskForAllButAcceptance</default>
      </entry>

    </group>


  <!-- FIXME: Make a separate setting for this for the composer and the reader. Only the
              reader setting should be here.
              Regression from r1021989.
  -->"
  <group name="Composer">
    <entry name="UseFixedFont" type="Bool" key="use-fixed-font">
      <label>Use Fi&amp;xed Font</label>
      <default>false</default>
    </entry>
  </group>
</kcfg>
