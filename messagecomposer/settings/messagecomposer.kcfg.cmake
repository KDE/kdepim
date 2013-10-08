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
      <label>Allow the semicolon character (';') to be used as separator in the message composer</label>
    </entry>
    <entry name="ShowRecentAddressesInComposer" type="Bool" key="showRecentAddressesInComposer">
      <label>Use recent addresses for autocompletion</label>
      <whatsthis>Disable this option if you do not want recently used addresses to appear in the autocompletion list in the composer's address fields.</whatsthis>
      <default>true</default>
    </entry>

    <entry name="MaximumRecipients" type="Int">
      <label>Maximum number of recipient entries:</label>
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
        <label>The maximum size in bits that email attachments are allowed to have (-1 for no limit)</label>
        <default>-1</default>
    </entry>
    <entry name="OutlookCompatibleAttachments" type="Bool" key="outlook-compatible-attachments">
      <label>Outlook-compatible attachment naming</label>
      <whatsthis>Turn this option on to make Outlook &#8482; understand attachment names containing non-English characters</whatsthis>
      <default>false</default>
    </entry>
    <entry name="WordWrap" type="Bool" key="word-wrap">
      <label>Word &amp;wrap at column:</label>
      <default>true</default>
    </entry>
    <entry name="LineWrapWidth" type="Int" key="break-at">
      <label></label>
      <default>78</default>
      <min>30</min>
      <max>998</max>
    </entry>
    <entry name="ImprovePlainTextOfHtmlMessage" type="Bool" key="improve-plain-text-html-message">
      <label>Improve plain text version of HTML message</label>
      <default>true</default>
    </entry>
    <entry name="CryptoWarningUnencrypted" type="Bool" key="crypto-warning-unencrypted">
        <label>Warn before sending unencrypted messages</label>
        <default>false</default>
    </entry>
    <entry name="CryptoWarningUnsigned" type="Bool" key="crypto-warning-unsigned">
        <label>Warn before sending unsigned messages</label>
        <default>false</default>
    </entry>
    <entry name="CryptoWarnRecvNotInCert" type="Bool" key="crypto-warn-recv-not-in-cert">
        <label>Warn if the receiver's address is not in the certificate</label>
        <default>true</default>
    </entry>
    <entry name="CryptoWarnWhenNearExpire" type="Bool" key="crypto-warn-when-near-expire">
        <label>Warn if certificates/keys expire soon (configure thresholds below)</label>
        <default>true</default>
    </entry>
    <entry name="CryptoWarnSignKeyNearExpiryThresholdDays" type="Int" key="crypto-warn-sign-key-near-expire-int">
        <label>The minimum number of days that the signature certificate should be valid before issuing a warning</label>
        <default>14</default>
    </entry>
    <entry name="CryptoWarnSignChaincertNearExpiryThresholdDays" type="Int" key="crypto-warn-sign-chaincert-near-expire-int">
        <label>The minimum number of days that the CA certificate should be valid before issuing a warning</label>
        <default>14</default>
    </entry>
    <entry name="CryptoWarnSignRootNearExpiryThresholdDays" type="Int" key="crypto-warn-sign-root-near-expire-int">
        <label>The minimum number of days that the root certificate should be valid before issuing a warning</label>
        <default>14</default>
    </entry>
    <entry name="CryptoWarnEncrKeyNearExpiryThresholdDays" type="Int" key="crypto-warn-encr-key-near-expire-int">
        <label>The minimum number of days that the encryption certificate should be valid before issuing a warning</label>
        <default>14</default>
    </entry>
    <entry name="CryptoWarnEncrChaincertNearExpiryThresholdDays" type="Int" key="crypto-warn-encr-chaincert-near-expire-int">
        <label>The minimum number of days that all certificates in the chain should be valid before issuing a warning</label>
        <default>14</default>
    </entry>
    <entry name="CryptoWarnEncrRootNearExpiryThresholdDays" type="Int" key="crypto-warn-encr-root-near-expire-int">
        <label>The minimum number of days that the root certificate should be valid before issuing a warning</label>
        <default>14</default>
    </entry>
      <entry name="CryptoEncryptToSelf" type="Bool" key="crypto-encrypt-to-self">
        <label>When encrypting emails, always also encrypt to the certificate of my own identity</label>
        <default>true</default>
      </entry>
      <entry name="CryptoShowKeysForApproval" type="Bool" key="crypto-show-keys-for-approval">
        <label>Always show the list of encryption keys to select the one which will be used</label>
        <default>true</default>
      </entry>
      <entry name="PgpAutoEncrypt" type="Bool" key="pgp-auto-encrypt">
        <default>false</default>
      </entry>
  </group>

  <group name="sending mail">
    <entry name="SendImmediate" type="Bool" key="Immediate">
      <default>true</default>
    </entry>
  </group>

  <group name="Autocorrect">
    <entry name="Enabled" type="Bool" key="enabled">
      <default>false</default>
    </entry>
    <entry name="UppercaseFirstCharOfSentence" type="Bool" key="upper-case-first-char-of-sentence">
      <default>false</default>
    </entry>
    <entry name="FixTwoUppercaseChars" type="Bool" key="fix-two-upper-case-chars">
      <default>false</default>
    </entry>
    <entry name="SingleSpaces" type="Bool" key="single-spaces">
      <default>false</default>
    </entry>
    <entry name="AutoFractions" type="Bool" key="auto-fractions">
      <default>false</default>
    </entry>
    <entry name="CapitalizeWeekDays" type="Bool" key="capitalize-week-days">
      <default>false</default>
    </entry>
    <entry name="AdvancedAutocorrect" type="Bool" key="advanced-autocorrect">
      <default>false</default>
    </entry>
    <entry name="ReplaceDoubleQuotes" type="Bool" key="replace-double-quotes">
      <default>false</default>
    </entry>
    <entry name="ReplaceSingleQuotes" type="Bool" key="replace-single-quotes">
      <default>false</default>
    </entry>
    <entry name="AutoFormatUrl" type="Bool" key="autoformat-url">
      <default>false</default>
    </entry>
    <entry name="AutoBoldUnderline" type="Bool" key="auto-bold-underline">
      <default>false</default>
    </entry>
    <entry name="SuperScript" type="Bool" key="super-script">
      <default>false</default>
    </entry>
    <entry name="AddNonBreakingSpaceInFrench" type="Bool" key="add-non-breaking-space">
      <default>false</default>
    </entry>
  </group>

  <group name="AutoResizeImage">
    <entry name="AutoResizeImageEnabled" type="Bool" key="auto-resize-image-enabled">
      <default>false</default>
    </entry>
    <entry name="KeepImageRatio" type="Bool" key="keep-image-ratio">
      <default>false</default>
    </entry>
    <entry name="AskBeforeResizing" type="Bool" key="ask-before-resizing">
      <default>true</default>
    </entry>
    <entry name="ReduceImageToMaximum" type="Bool" key="reduce-image-to-maximum">
      <default>false</default>
    </entry>
    <entry name="EnlargeImageToMinimum" type="Bool" key="enlarge-image-to-minimum">
      <default>false</default>
    </entry>
    <entry name="CustomMinimumHeight" type="int" key="custom-minimum-height">
      <default>520</default>
    </entry>
    <entry name="CustomMinimumWidth" type="int" key="custom-minimum-width">
      <default>520</default>
    </entry>
    <entry name="CustomMaximumHeight" type="int" key="custom-maximum-height">
      <default>520</default>
    </entry>
    <entry name="CustomMaximumWidth" type="int" key="custom-maximum-width">
      <default>520</default>
    </entry>

    <entry name="MaximumHeight" type="int" key="maximum-height">
      <default>0</default>
    </entry>
    <entry name="MaximumWidth" type="int" key="maximum-width">
      <default>0</default>
    </entry>
    <entry name="MinimumHeight" type="int" key="minimum-height">
      <default>0</default>
    </entry>
    <entry name="MinimumWidth" type="int" key="minimum-width">
      <default>0</default>
    </entry>
    <entry name="WriteFormat" type="String" key="write-format">
      <default></default>
    </entry>
    <entry name="SkipImageLowerSizeEnabled" type="Bool" key="skip-image-lower-size-enabled">
      <default>false</default>
    </entry>
    <entry name="SkipImageLowerSize" type="int" key="skip-image-lower-size">
      <default>220</default>
    </entry>

    <entry name="FilterSourcePattern" type="String" key="filter-source-pattern">
      <default></default>
    </entry>

    <entry name="FilterSourceType" type="Enum">
      <label></label>
      <choices>
        <choice name="NoFilter"/>
        <choice name="IncludeFilesWithPattern"/>
        <choice name="ExcludeFilesWithPattern"/>
      </choices> 
      <default>NoFilter</default>
    </entry>

    <entry name="RenameResizedImages" type="Bool" key="rename-resized-images">
      <default>false</default>
    </entry>
    <entry name="RenameResizedImagesPattern" type="String" key="rename-resized-images-pattern">
      <default></default>
    </entry>  
    <entry name="FilterRecipientType" type="Enum">
      <label></label>
      <choices>
        <choice name="NoFilter"/>
        <choice name="ResizeEachEmailsContainsPattern"/>
        <choice name="ResizeOneEmailContainsPattern"/>
        <choice name="DontResizeEachEmailsContainsPattern"/>
        <choice name="DontResizeOneEmailContainsPattern"/>
      </choices>
      <default>NoFilter</default>
    </entry>
    <entry name="DoNotResizeEmailsPattern" type="String" key="do-not-resize-emails-pattern">
      <default></default>
    </entry>
    <entry name="ResizeEmailsPattern" type="String" key="resize-emails-pattern">
      <default></default>
    </entry>
    <entry name="ResizeImagesWithFormats" type="Bool" key="resize-image-with-formats">
      <default>false</default>
    </entry>
    <entry name="ResizeImagesWithFormatsType" type="String" key="resize-image-with-formats-type">
      <default></default>
    </entry>

  </group>
</kcfg>

