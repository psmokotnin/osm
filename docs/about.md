---
# Feel free to add content and custom Front Matter to this file.
# To modify the layout, see https://jekyllrb.com/docs/themes/#overriding-theme-defaults

layout: default
title: About Open Sound Meter
---
  
<!-- Marketing messaging and featurettes
================================================== -->
<!-- Wrap the rest of the page in another container to center all the content. -->


<div class="container marketing">

    <div class="row featurette">
      <div class="col-md-7 order-md-2">
        <h2 class="featurette-heading mt-md-0">About the project</h2>
        <p class="">
            Open Sound Meter was concieved as new approach to audio engineering software. My goal was to make it 
            in a completely different way: very simple, intuitive and fast.
        </p>
        <p class="">
            I started the development in 2017, the first version of Open Sound Meter was released in 2018.
            Most of the time was spent looking for the best mathematical solutions in order for the program to work correctly, 
            quickly and stably, and without excessive CPU load. 
            I came up with a several innovative ideas (data low pass filters, log time windows transform, etc)
        </p>
        <p class="">
            Desktop version is open-source, you can contribute to the project on <a href="https://github.com/psmokotnin/osm">GitHub</a>!
        </p>
        <p class="">
            Features planned for future implementation:
            <ul>
                <li>SPL meters</li>
                <li>Sweep tone measurements</li>
            </ul>
        </p>
        <p class="">
            Available in App Store since July 5, 2021.
        </p>
        <p>
            <a class="download ios" target="_blank" href="https://apps.apple.com/app/id1552933259"><img style="width: 100px;" src="/images/icons/appstore.svg" class="bd-placeholder-img"/></a>
        </p>
        
      </div>
      <div class="col-md-5 order-md-1">
        <img src="/images/screens/v0.2.average.png" class="bd-placeholder-img bd-placeholder-img-lg featurette-image img-fluid mx-auto"/>
      </div>
    </div>

    <hr class="featurette-divider">
    
    <div class="row featurette">
      <div class="col-md-7 order-md-2">
        <h2 class="featurette-heading mt-md-0">Developer</h2>
        <p class="">
            My name is Pavel Smokotnin, I am an engineer by education, my passion is sound and higher math.<br/><br/>
            I have extensive experience as a sound engineer. That's why I know exactly how the product should work and what functions it should have.<br/><br/>
            I am developing other audio applications and devices as a job.<br/><br/>
            Feel free to contact me:<br/><br/>
            <a href="https://www.facebook.com/psmokotnin">Facebook</a> &bull; <a href="https://www.linkedin.com/in/psmokotnin/">LinkedIn</a>
        </p>
      </div>
      <div class="col-md-5 order-md-1">
        <img src="/images/pavel.png" class="bd-placeholder-img bd-placeholder-img-lg featurette-image img-fluid mx-auto"/>
      </div>
    </div>
    

    <hr class="featurette-divider">
    
    {% include donate.html %}
    
</div>

<script>
$('.download.ios').click(function() {
    gtag('event', 'appstore', {
        'os' : 'ipad'
    });
    return true;
});
</script>