---
# Feel free to add content and custom Front Matter to this file.
# To modify the layout, see https://jekyllrb.com/docs/themes/#overriding-theme-defaults

layout: default
---


<h1 class="page-title">{{ page.title | escape }}</h1>

<div class="section">
    <div class="row">
        <div class="col s6 offset-s3">
            <p class="light">Hello to the Open Sound Meter project, real time dual-FFT measurement analysis tool for sound system.</p>
        </div>
    </div>
</div>
<div class="section">
    <div class="row">
          <div class="carousel carousel-slider center" data-indicators="true" data-namespace="DIVcarouselcarousel-slider1" style="height: 400px;">
          <div class="carousel-fixed-item center with-indicators">
          </div>
          
          {% for image in site.data.carousel %}
            <div class="carousel-item" style="background: url({{ image.src }}) center center / contain no-repeat;">
                <h2>{{ image.title }}</h2>
                <p>{{ image.description }}</p>
            </div>
          {% endfor %}
          
    </div>
</div>

    <div class="row center">
        <a href="{{ site.download_page }}" id="download-button" class="btn-large waves-effect waves-light orange">Download</a>
    </div>
    
  <div class="container">
    <div class="section">
        <div class="row">
            
            <div class="col s12 m4">
                <div class="icon-block">
                    <h2 class="center light-blue-text"><i class="material-icons">mood</i></h2>
                    <h5 class="center">Simple & easy</h5>
                    <p class="light">You do not need to search functions in the thousands of menus. All required is placed just in front of you. The main goal is to speed up a system's tune at small and medium venues.</p>
                </div>
            </div>
        
            <div class="col s12 m4">
                <div class="icon-block">
                    <h2 class="center light-blue-text"><i class="material-icons">money_off</i></h2>
                    <h5 class="center">Free & open source</h5>
                    <p class="light">Software distributed under GPL-3.0 license. You don't have to pay any fixed price. If you like it, please support me.</p>
                    <p>
                        <a class="waves-effect waves-light btn" href="{{ site.payme_page }}" target="_blank">Pay what you want</a>
                    </p>
                </div>
            </div>            

            <div class="col s12 m4">
                <div class="icon-block">
                    <h2 class="center light-blue-text"><i class="material-icons">desktop_mac</i></h2>
                    <h5 class="center">Crossplatform</h5>
                    <p class="light">{{site.title}} is easy to build on Windows, MacOS or Linux. It's developed in C++ with Qt SDK and no additional libraries.</p>
                    <p>
                        <a class="waves-effect waves-light btn orange" href="{{ site.source_page }}" target="_blank">Github</a>
                    </p>
                </div>
            </div>
            
        </div>
    </div>
  </div>