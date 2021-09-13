---
# Feel free to add content and custom Front Matter to this file.
# To modify the layout, see https://jekyllrb.com/docs/themes/#overriding-theme-defaults

layout: default
title: Open Sound Meter
---
  
<!-- Marketing messaging and featurettes
================================================== -->
<!-- Wrap the rest of the page in another container to center all the content. -->

<div class="container marketing">

    <div class="row featurette">
      <div class="col-md-7 order-md-2">
        <h2 class="featurette-heading">Open Sound Meter</h2>
        <p class="lead">
            Sound measurement application for tuning audio systems in real-time <br/>
            by Pavel Smokotnin
        </p>
        
        <div class="">
            <div class="row">
                <div class="col-md-auto">
                    <b>The modern all-around sound analysis app for iPad!</b><br/>
                    Available on App Store from July 5, 2021
                </div>
                <div class="col" style="padding-top: 3px;">
                    <a class="download ios" target="_blank" href="https://apps.apple.com/app/id1552933259"><img src="/images/icons/appstore.svg" class="bd-placeholder-img"/></a>
                </div>
            </div>
        </div>
      </div>
      <div class="col-md-5 order-md-1">
        <img src="/images/screens/main.png" class="bd-placeholder-img bd-placeholder-img-lg featurette-image img-fluid mx-auto"/>
      </div>
    </div>

    <hr class="featurette-divider">
    
<!-- Three columns of text below the carousel -->
    <div class="row">
        <div class="col-lg-4">
            <img src="/images/icons/abc.png" class="bd-placeholder-img" width="140" height="140"/>
            <h2>Simple</h2>
            <p align="left">Easy to start. Uncomplicated interface with only needed functions.</p>
            <p><a class="btn btn-info" href="/download" role="button">Download</a></p>
        </div><!-- /.col-lg-4 -->
        
        <div class="col-lg-4">
            <div style="height: 105px;margin-top: 35px;"><img src="/images/icons/heart.svg" class="bd-placeholder-img" width="70" height="70"/></div>
            <h2>Available</h2>
            <p align="left">Pay what you want for desktop version. Every donation is a great help!</p>
            <p><a class="btn btn-success" href="/about#donate" role="button">Donate</a></p>
        </div><!-- /.col-lg-4 -->
        
        <div class="col-lg-4">
            <div class="container brand-images ">
                <div class="row">
                    <div class="col"><img src="/images/icons/apple-brands.svg" class="bd-placeholder-img"/></div>
                    <div class="col"><img src="/images/icons/windows-brands.svg" class="bd-placeholder-img"/></div>
                    <div class="col"><img src="/images/icons/linux-brands.svg" class="bd-placeholder-img"/></div>
                </div>
            </div>
            <h2>Crossplatform</h2>
            <p align="left">Desktop version are available for macOS, Windows, and Linux.</p>
            <p><a class="btn btn-info" href="/download" role="button">Download</a></p>
        </div><!-- /.col-lg-4 -->
    </div><!-- /.row -->


    <!-- START THE FEATURETTES -->

    <hr class="featurette-divider">

    <div class="row featurette">
      <div class="col-md-7">
        <h2 class="featurette-heading">All basic functions</h2>
        <p class="lead">RTA, magnitude, phase and impulse response, coherence, group delay<!--  and spectrogram --></p>
      </div>
      <div class="col-md-5">
        <img src="/images/screens/basic.png" class="bd-placeholder-img bd-placeholder-img-lg featurette-image img-fluid mx-auto"/>
      </div>
    </div>

    <hr class="featurette-divider">

    <div class="row featurette">
      <div class="col-md-7 order-md-2">
        <h2 class="featurette-heading">Keep your workspace clean</h2>
        <p class="lead">You can save and open your measurement projects. Each store automatically collects measurement settings.</p>
      </div>
      <div class="col-md-5 order-md-1">
        <img src="/images/screens/clean.png" class="bd-placeholder-img bd-placeholder-img-lg featurette-image img-fluid mx-auto"/>
      </div>
    </div>

    <hr class="featurette-divider">

    <div class="row featurette">
      <div class="col-md-7">
        <h2 class="featurette-heading">And something more</h2>
        <p class="lead">
            Additional features: 
            <ul>
                <li>Wavelength calculator</li>
<!--                 <li>virtual summation, difference or averaging of up to 4 measurements and stores</li> -->
                <li>signal generator</li>
                <li>automatic check for update</li>
            </ul>
        </p>
      </div>
      <div class="col-md-5">
        <img src="/images/screens/sum.png" class="bd-placeholder-img bd-placeholder-img-lg featurette-image img-fluid mx-auto"/>
      </div>
    </div>

    <!-- /END THE FEATURETTES -->

  </div><!-- /.container -->
  
<script>
$('.download.ios').click(function() {
    gtag('event', 'appstore', {
        'os' : 'ipad'
    });
    return true;
});
</script>
