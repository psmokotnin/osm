---
# Feel free to add content and custom Front Matter to this file.
# To modify the layout, see https://jekyllrb.com/docs/themes/#overriding-theme-defaults

layout: default
title: Download Open Sound Meter
---
  
<!-- Marketing messaging and featurettes
================================================== -->
<!-- Wrap the rest of the page in another container to center all the content. -->

<div class="container marketing">
    <div class="row">
        <div class="col">
            <div class="card-deck">
        
                <a class="card border-info download ios" style="padding: 2em 4em;" href="https://apps.apple.com/app/id1552933259" target="_blank">
                    <img class="card-img-top" src="/images/icons/appstore.svg" alt="Card image cap" style="max-height: 200px;margin: auto;padding: 50px 0;">
                    <div class="card-body text-center">
                        <h5 class="card-title">iPad</h5>
                        <p class="card-text"></p>
                    </div>
                </a>
                
                <a class="card download mac" href="https://github.com/psmokotnin/osm/releases/download/{{site.current_version}}/OpenSoundMeter.dmg" target="_blank">
                    <img class="card-img-top" src="/images/icons/apple-brands.svg" alt="Card image cap" style="max-height: 200px;margin: auto;padding: 50px 0;">
                    <div class="card-body text-center">
                        <h5 class="card-title">macOS</h5>
                        <p class="card-text">macOS 10.13 - 11.1</p>
                    </div>
                </a>
                
                <a class="card download windows" href="https://github.com/psmokotnin/osm/releases/download/{{site.current_version}}/setupOSM.exe" target="_blank">
                    <img class="card-img-top" src="/images/icons/windows-brands.svg" alt="Card image cap" style="max-height: 200px;margin: auto;padding: 50px 0;">
                    <div class="card-body text-center">
                        <h5 class="card-title">Windows</h5>
                        <p class="card-text">from windows 7</p>
                    </div>
                </a>
                
                <a class="card download linux" href="https://github.com/psmokotnin/osm/releases/download/v1.0.1/Open_Sound_Meter-v1.0.1-x86_64.AppImage" target="_blank">
                    <img class="card-img-top" src="/images/icons/linux-brands.svg" alt="Card image cap" style="max-height: 200px;margin: auto;padding: 50px 0;">
                    <div class="card-body text-center">
                        <h5 class="card-title">Linux</h5>
                        <p class="card-text">AppImage1</p>
                    </div>
                </a>
            </div>
        </div>
    </div>
    
    <hr class="featurette-divider">
    
    {% include donate.html %}
    
</div>
<script>
$('.download.mac').click(function() {
    gtag('event', 'downloadmacos', {
        'os' : 'macos'
    });
    return true;
});
$('.download.windows').click(function() {
    gtag('event', 'downloadwindows', {
        'os' : 'windows'
    });
    return true;
});
$('.download.linux').click(function() {
    gtag('event', 'downloadlinux', {
        'os' : 'linux'
    });
    return true;
});
$('.download.ios').click(function() {
    gtag('event', 'appstore', {
        'os' : 'ipad'
    });
    return true;
});
</script>